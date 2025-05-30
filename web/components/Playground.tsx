import * as React from "react";
import * as TS from "web-tree-sitter";
import ThemeToggle from "./ThemeToggle";
import LanguageSelect from "./LanguageSelect";
import QueryContainer from "./QueryContainer";
import PanelHeader from "./PanelHeader";
import OutputContainer from "./OutputContainer";
import CodeContainer from "./CodeContainer";

const Playground: React.FC = () => {
  const [visible, setVisible] = React.useState<boolean>(false);
  const parserRef = React.useRef<TS.Parser>(null);
  const [language, setLanguage] = React.useState<string | null>(null);
  const [tree, setTree] = React.useState<TS.Tree | null>(null);
  const [code, setCode] = React.useState<string>("");

  React.useEffect(() => {
    setLanguage("moonbit");
  }, []);

  React.useEffect(() => {
    TS.Parser.init().then(() => {
      setVisible(true);
      parserRef.current = new TS.Parser();
    });
  }, []);

  React.useEffect(() => {
    if (parserRef.current) {
      TS.Language.load(`tree-sitter-${language}.wasm`).then((language) => {
        if (!parserRef.current) {
          console.error("Parser is not initialized");
          return;
        }
        parserRef.current.setLanguage(language);
      });
    }
  }, [language]);

  React.useEffect(() => {
    if (parserRef.current && code) {
      const startTime = performance.now();
      console.log("Parsing code:", code);
      const newTree = parserRef.current.parse(code);
      console.log("Parsed tree:", newTree);
      const endTime = performance.now();
      setTree(newTree);
      document.getElementById("update-time")!.textContent = `${(
        endTime - startTime
      ).toFixed(2)} ms`;
    } else {
      setTree(null);
    }
  }, [code]);

  return (
    <div
      id="playground-container"
      style={{ visibility: visible ? "visible" : "hidden" }}
    >
      <header>
        <div className="header-item">
          <span className="language-name">Language: MoonBit</span>
        </div>

        <div className="header-item">
          <input id="logging-checkbox" type="checkbox" />
          <label htmlFor="logging-checkbox">log</label>
        </div>

        <div className="header-item">
          <input id="anonymous-nodes-checkbox" type="checkbox" />
          <label htmlFor="anonymous-nodes-checkbox">show anonymous nodes</label>
        </div>

        <div className="header-item">
          <input id="query-checkbox" type="checkbox" />
          <label htmlFor="query-checkbox">query</label>
        </div>

        <div className="header-item">
          <input id="accessibility-checkbox" type="checkbox" />
          <label htmlFor="accessibility-checkbox">accessibility</label>
        </div>

        <div className="header-item">
          <label htmlFor="update-time">parse time: </label>
          <span id="update-time"></span>
        </div>

        <div className="header-item">
          <a href="https://tree-sitter.github.io/tree-sitter/7-playground.html#about">
            (?)
          </a>
        </div>

        <div className="header-item">
          <LanguageSelect
            languages={[
              { name: "MoonBit", value: "moonbit" },
              { name: "MBTQ", value: "mbtq" },
            ]}
            onChange={(language) => {
              setLanguage(language);
            }}
          />
        </div>

        <div className="header-item">
          <ThemeToggle />
        </div>
      </header>

      <main>
        <div id="input-pane">
          <PanelHeader>Code</PanelHeader>
          <CodeContainer
            value={code}
            onChange={(code) => {
              console.log("Code changed:", code);
              setCode(code);
            }}
          />

          <QueryContainer />
        </div>

        <div id="output-container-scroll">
          <PanelHeader>Tree</PanelHeader>
          <OutputContainer tree={tree} />
        </div>
      </main>
    </div>
  );
};

export default Playground;
