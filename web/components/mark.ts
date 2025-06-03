import * as CMState from "@codemirror/state";
import * as CMView from "@codemirror/view";

export const Effect = CMState.StateEffect.define<{
  from: number;
  to: number;
  style: string;
}>({
  map: (value, mapping) => ({
    from: mapping.mapPos(value.from, -1),
    to: mapping.mapPos(value.to, 1),
    style: value.style,
  }),
});

export const Field = CMState.StateField.define<CMView.DecorationSet>({
  create() {
    return CMView.Decoration.none;
  },
  update(value, tr) {
    value = value.map(tr.changes);
    for (const effect of tr.effects) {
      if (effect.is(Effect)) {
        const { from, to, style } = effect.value;
        const decoration = CMView.Decoration.mark({
          attributes: {
            style,
          },
        });
        value = value.update({
          add: [decoration.range(from, to)],
        });
      }
    }
    return value;
  },
});
