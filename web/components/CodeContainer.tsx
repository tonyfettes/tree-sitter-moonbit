import * as React from "react";
import CodeMirror from "./CodeMirror";
import * as CMState from "@codemirror/state";

type CodeContainerProps = {
  value: string;
  selections?: readonly CMState.SelectionRange[];
  onChange?: (code: string) => void;
  onSelect?: (selections: readonly CMState.SelectionRange[]) => void;
};

const CodeContainer: React.FC<CodeContainerProps> = ({
  value,
  selections,
  onChange,
  onSelect,
}) => {
  return (
    <div id="code-container">
      <CodeMirror
        value={value}
        selections={selections}
        onChange={onChange}
        onSelect={onSelect}
      />
    </div>
  );
};

export default CodeContainer;
