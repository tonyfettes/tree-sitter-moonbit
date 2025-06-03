import CodeMirror from "./CodeMirror.tsx";
import * as CMState from "@codemirror/state";
import * as React from "react";

const LIGHT_COLORS = [
  "#0550ae", // blue
  "#ab5000", // rust brown
  "#116329", // forest green
  "#844708", // warm brown
  "#6639ba", // purple
  "#7d4e00", // orange brown
  "#0969da", // bright blue
  "#1a7f37", // green
  "#cf222e", // red
  "#8250df", // violet
  "#6e7781", // gray
  "#953800", // dark orange
  "#1b7c83", // teal
];

const DARK_COLORS = [
  "#79c0ff", // light blue
  "#ffa657", // orange
  "#7ee787", // light green
  "#ff7b72", // salmon
  "#d2a8ff", // light purple
  "#ffa198", // pink
  "#a5d6ff", // pale blue
  "#56d364", // bright green
  "#ff9492", // light red
  "#e0b8ff", // pale purple
  "#9ca3af", // gray
  "#ffb757", // yellow orange
  "#80cbc4", // light teal
];

const CAPTURE_REGEX = /@\s*([\w._-]+)/g;

function colorForCaptureName(id: number) {
  const isDark =
    document.querySelector("html")?.classList.contains("ayu") ||
    document.querySelector("html")?.classList.contains("coal") ||
    document.querySelector("html")?.classList.contains("navy") ||
    false;

  const colors = isDark ? DARK_COLORS : LIGHT_COLORS;
  return colors[id % colors.length];
}

type QueryContainerProps = {
  visible?: boolean;
  value: string;
  onChange?: (query: string) => void;
};

const QueryContainer: React.FC<QueryContainerProps> = ({
  visible,
  value,
  onChange,
}) => {
  const [markers, setMarkers] = React.useState<
    { range: CMState.SelectionRange; style: string }[]
  >([]);

  const style: React.CSSProperties = {};
  if (!visible) {
    style.visibility = "hidden";
    style.position = "absolute";
  }

  React.useEffect(() => {
    const lines = value.split("\n");
    const newMarkers: { range: CMState.SelectionRange; style: string }[] = [];
    let position = 0;
    for (const line of lines) {
      const match = CAPTURE_REGEX.exec(line);
      if (!match) {
        continue;
      }
      newMarkers.push({
        range: CMState.EditorSelection.range(
          position + match.index,
          position + match.index + match[0].length
        ),
        style: `color: ${colorForCaptureName(newMarkers.length)}`,
      });
      position += line.length + 1; // +1 for the newline character
    }
    setMarkers(newMarkers);
  }, [value]);

  return (
    <div id="query-container" style={style}>
      <div className="panel-header">Query</div>
      <CodeMirror value={value} onChange={onChange} markers={markers} />
    </div>
  );
};

export default QueryContainer;
