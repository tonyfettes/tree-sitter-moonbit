import * as CM from "codemirror";
import * as CMState from "@codemirror/state";
import * as React from "react";
import * as Mark from "./mark";

type CodeMirrorProps = {
  value: string;
  selections?: readonly CMState.SelectionRange[];
  markers?: readonly { range: CMState.SelectionRange; style: string }[];
  onChange?: (value: string) => void;
  onSelect?: (selections: readonly CMState.SelectionRange[]) => void;
};

const CodeMirror: React.FC<CodeMirrorProps> = ({
  value,
  selections,
  markers,
  onChange,
  onSelect,
}) => {
  const editorRef = React.useRef<CM.EditorView | null>(null);
  const parentRef = React.useRef<HTMLDivElement | null>(null);

  React.useEffect(() => {
    if (editorRef.current) {
      return;
    }
    editorRef.current = new CM.EditorView({
      doc: "",
      extensions: [CM.basicSetup, Mark.Field],
      parent: parentRef.current!,
    });
  }, []);

  React.useEffect(() => {
    if (!editorRef.current) {
      return;
    } else {
      editorRef.current.dispatch({
        effects: CMState.StateEffect.reconfigure.of([
          CM.basicSetup,
          Mark.Field,
          CM.EditorView.updateListener.of((update) => {
            if (update.docChanged && onChange) {
              onChange(update.state.doc.toString());
            }
            if (update.selectionSet && onSelect) {
              const selections = update.state.selection.ranges;
              onSelect(selections);
            }
          }),
        ]),
      });
    }
  }, [onChange, onSelect]);

  React.useEffect(() => {
    if (!editorRef.current) {
      return;
    }
    if (value === editorRef.current.state.doc.toString()) {
      return;
    }
    editorRef.current.dispatch({
      changes: {
        from: 0,
        to: editorRef.current.state.doc.length,
        insert: value,
      },
    });
  }, [value]);

  React.useEffect(() => {
    if (!editorRef.current) {
      return;
    }
    if (selections === undefined) {
      return;
    }
    const sortedSelections = [...selections];
    sortedSelections.sort((a, b) => {
      const aStart = a.from;
      const bStart = b.from;
      if (aStart === bStart) {
        return a.to - b.to; // If starts are equal, sort by end position
      } else {
        return aStart - bStart;
      }
    });
    let equals = true;
    if (
      editorRef.current.state.selection.ranges.length !==
      sortedSelections.length
    ) {
      equals = false;
    } else {
      for (let i = 0; i < sortedSelections.length; i++) {
        const a = editorRef.current.state.selection.ranges[i];
        const b = sortedSelections[i];
        if (a.from !== b.from || a.to !== b.to) {
          equals = false;
          break;
        }
      }
    }
    if (!equals) {
      const selection = CMState.EditorSelection.create(sortedSelections);
      editorRef.current.dispatch({
        selection,
      });
      editorRef.current.scrollDOM.scrollTo({
        top: editorRef.current.lineBlockAt(selection.main.from).top,
        behavior: "smooth",
      });
    }
  }, [selections]);

  React.useEffect(() => {
    if (!editorRef.current || !markers) {
      return;
    }
    const effects = markers.map((marker) => {
      return Mark.Effect.of({
        from: marker.range.from,
        to: marker.range.to,
        style: marker.style,
      });
    });
    editorRef.current.dispatch({
      effects,
    });
  }, [markers]);

  return <div ref={parentRef} />;
};

export default CodeMirror;
