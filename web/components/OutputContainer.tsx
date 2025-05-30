import * as TS from "web-tree-sitter";
import * as React from "react";
import TreeRow from "./TreeRow";

type OutputContainerProps = {
  tree: TS.Tree | null;
};

const OutputContainer: React.FC<OutputContainerProps> = ({ tree }) => {
  return (
    <pre className="highlight">
      {tree ? (
        <TreeRow
          indent={0}
          field={""}
          node={tree.rootNode}
          highlighted={new Map()}
        />
      ) : (
        "No tree available"
      )}
    </pre>
  );
};

export default OutputContainer;
