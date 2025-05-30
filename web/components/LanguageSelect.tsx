import * as React from "react";

type LanguageSelectProps = {
  languages: { name: string; value: string }[];
  onChange?: (language: string) => void;
};

const LanguageSelect: React.FC<LanguageSelectProps> = ({
  languages,
  onChange,
}) => {
  const [selectedLanguage, setSelectedLanguage] = React.useState<string>(
    languages[0].value
  );
  return (
    <select
      value={selectedLanguage}
      onChange={(e) => {
        setSelectedLanguage(e.target.value);
        onChange?.(e.target.value);
      }}
    >
      {languages.map((language) => (
        <option key={language.value} value={language.value}>
          {language.name}
        </option>
      ))}
    </select>
  );
};

export default LanguageSelect;
