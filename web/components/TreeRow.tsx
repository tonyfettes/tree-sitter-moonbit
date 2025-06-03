import * as TS from "web-tree-sitter";
import * as React from "react";

type TreeRowProps = {
  indent: number;
  field: string | null;
  node: TS.Node;
  highlighted: Map<number, boolean>;
  onClick?: (nodeId: number, startIndex: number, endIndex: number) => void;
};

const TreeRow: React.FC<TreeRowProps> = ({
  indent,
  field,
  node,
  highlighted,
  onClick,
}) => {
  let className = node.isError
    ? "node-link error"
    : node.isMissing
    ? "node-link missing"
    : node.isNamed
    ? "node-link named"
    : "node-link anonymous";
  if (highlighted.has(node.id)) {
    className += " highlighted";
  }
  const displayName = node.isMissing
    ? node.isNamed
      ? node.type
      : `"${node.type}"`
    : node.type;
  const fieldName = field ? `${field}: ` : "";
  const startPosition = node.startPosition;
  const endPosition = node.endPosition;
  const dataRange = `${startPosition.row},${startPosition.column},${endPosition.row},${endPosition.column}`;
  return (
    <div className="tree-row">
      {"  ".repeat(indent)}
      {fieldName}
      <a
        className={className}
        href="#"
        data-id={node.id}
        data-range={dataRange}
        onClick={(e) => {
          e.preventDefault();
          console.log(
            `Clicked node: ${node.id}, range: ${dataRange}, type: ${node.type}`
          );
          onClick?.(node.id, node.startIndex, node.endIndex);
        }}
      >
        {displayName}
      </a>
      <span className="position-info">
        [{startPosition.row}, {startPosition.column}] - [{endPosition.row},{" "}
        {endPosition.column}]
      </span>
      {node.children.length > 0 &&
        node.children.map((child, index) => {
          return (
            child && (
              <TreeRow
                key={`${node.id}-${index}`}
                indent={indent + 1}
                field={node.fieldNameForChild(index)}
                node={child}
                highlighted={highlighted}
                onClick={onClick}
              />
            )
          );
        })}
    </div>
  );
};

export default TreeRow;
