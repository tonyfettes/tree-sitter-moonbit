import CodeMirror from "./CodeMirror.tsx";
import * as React from "react";

type QueryContainerProps = {
  onChange?: (query: string) => void;
};

const QueryContainer: React.FC<QueryContainerProps> = ({ onChange }) => {
  return (
    <div
      id="query-container"
      style={{ visibility: "hidden", position: "absolute" }}
    >
      <div className="panel-header">Query</div>
      <CodeMirror value="" onChange={onChange} />
    </div>
  );
};

export default QueryContainer;
