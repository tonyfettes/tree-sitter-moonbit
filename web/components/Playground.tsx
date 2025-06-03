import * as React from "react";
import * as TS from "web-tree-sitter";
import ThemeToggle from "./ThemeToggle";
import LanguageSelect from "./LanguageSelect";
import QueryContainer from "./QueryContainer";
import PanelHeader from "./PanelHeader";
import OutputContainer from "./OutputContainer";
import CodeContainer from "./CodeContainer";
import * as CMState from "@codemirror/state";
import Checkbox from "./Checkbox";

const Playground: React.FC = () => {
  const [visible, setVisible] = React.useState<boolean>(false);
  const parserRef = React.useRef<TS.Parser>(null);
  const [language, setLanguage] = React.useState<string>("moonbit");
  const [tree, setTree] = React.useState<TS.Tree | null>(null);
  const [code, setCode] = React.useState<string>("");
  const [selections, setSelections] = React.useState<
    readonly CMState.SelectionRange[] | undefined
  >(undefined);
  const [queryEnabled, setQueryEnabled] = React.useState<boolean>(false);
  const [queryText, setQueryText] = React.useState<string>("");
  const [query, setQuery] = React.useState<TS.Query | null>(null);

  React.useEffect(() => {
    if (parserRef.current) {
      TS.Language.load(`tree-sitter-${language}.wasm`).then((language) => {
        if (!parserRef.current) {
          console.error("Parser is not initialized");
          return;
        }
        parserRef.current.setLanguage(language);
      });
    } else {
      TS.Parser.init().then(() => {
        setVisible(true);
        parserRef.current = new TS.Parser();
        TS.Language.load(`tree-sitter-${language}.wasm`).then((language) => {
          if (!parserRef.current) {
            console.error("Parser is not initialized");
            return;
          }
          parserRef.current.setLanguage(language);
          console.log("Parser initialized with language:", language.name);
        });
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

  React.useEffect(() => {
    if (parserRef.current && queryText) {
      const language = parserRef.current.language;
      if (!language) {
        return;
      }
      try {
        setQuery(new TS.Query(language, queryText));
      } catch (error) {
        console.error("Error creating query:", error);
        setQuery(null);
      }
    }
  }, [queryText]);

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
          <Checkbox value={queryEnabled} onChange={setQueryEnabled}>
            query
          </Checkbox>
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
            selections={selections}
            onChange={setCode}
            onSelect={setSelections}
          />

          <QueryContainer
            value={queryText}
            onChange={setQueryText}
            visible={queryEnabled}
          />
        </div>

        <div id="output-container-scroll">
          <PanelHeader>Tree</PanelHeader>
          <OutputContainer
            tree={tree}
            selections={selections}
            onClick={(nodeId, startIndex, endIndex) => {
              console.log("Node clicked:", nodeId, startIndex, endIndex);
              setSelections([
                CMState.EditorSelection.range(startIndex, endIndex),
              ]);
            }}
          />
        </div>
      </main>
    </div>
  );
};

export default Playground;
