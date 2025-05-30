import * as React from "react";

const PanelHeader: React.FC<{ children: string }> = ({ children }) => {
  return <div className="panel-header">{children}</div>;
};

export default PanelHeader;
