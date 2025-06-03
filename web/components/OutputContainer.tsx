import * as TS from "web-tree-sitter";
import * as React from "react";
import * as CMState from "@codemirror/state";
import TreeRow from "./TreeRow";

type OutputContainerProps = {
  tree: TS.Tree | null;
  selections?: readonly CMState.SelectionRange[];
  onClick?: (nodeId: number, startIndex: number, endIndex: number) => void;
};

const OutputContainer: React.FC<OutputContainerProps> = ({
  tree,
  selections,
  onClick,
}) => {
  const highlighted = new Map<number, boolean>();
  for (const selection of selections || []) {
    if (selection.from === selection.to) {
      continue;
    }
    const descendant = tree?.rootNode.descendantForIndex(
      selection.from,
      selection.to
    );
    if (descendant) {
      highlighted.set(descendant.id, true);
    }
  }
  return (
    <pre id="output-container" className="highlight">
      {tree ? (
        <TreeRow
          indent={0}
          field={""}
          node={tree.rootNode}
          highlighted={highlighted}
          onClick={onClick}
        />
      ) : (
        "No tree available"
      )}
    </pre>
  );
};

export default OutputContainer;
