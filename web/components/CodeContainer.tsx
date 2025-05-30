import * as React from "react";
import CodeMirror from "./CodeMirror";

type CodeContainerProps = {
  value: string;
  onChange?: (code: string) => void;
};

const CodeContainer: React.FC<CodeContainerProps> = ({ value, onChange }) => {
  return (
    <div id="code-container">
      <CodeMirror value={value} onChange={onChange} />
    </div>
  );
};

export default CodeContainer;
