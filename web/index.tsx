import * as React from "react";
import * as ReactDOM from "react-dom/client";
import Playground from "./components/Playground";

document.addEventListener("DOMContentLoaded", () => {
  ReactDOM.createRoot(document.getElementById("playground")!).render(
    <React.StrictMode>
      <Playground />
    </React.StrictMode>
  );
});
