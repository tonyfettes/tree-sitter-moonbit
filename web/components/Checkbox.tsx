import * as React from "react";

type CheckboxProps = {
  value: boolean;
  onChange: (value: boolean) => void;
  children: string;
};

const Checkbox: React.FC<CheckboxProps> = ({ value, onChange, children }) => {
  const inputId = React.useId();
  return (
    <div>
      <input
        type="checkbox"
        id={inputId}
        checked={value}
        onChange={(e) => onChange(e.target.checked)}
      />
      <label htmlFor={inputId}>{children}</label>
    </div>
  );
};

export default Checkbox;
