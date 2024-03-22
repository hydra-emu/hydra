## Hydra core specific settings
Hydra cores use toml format to generate widgets that appear in the GUI at runtime.

Each widget is a toml table like so:
```toml
[my_setting_name]
name = "My important BIOS file!"
description = "We need this BIOS file please provide it!"
category = "BIOS files"
type = "filepicker"
extensions = "bin,bios"
required = true
```

Hydra then uses these widgets to store core specific settings in a settings.json

`my_setting_name` is for hydra to identify this setting. The actual name becomes <CoreName>_my_setting_name
where <CoreName> is the current core name.

`name` is the pretty name of the setting that appears in the GUI.

`description` is a description that appears when you hover the widget.

`category` is the group this widget will appear in. It can be any value and widgets with the same category will be grouped together.

`type` is the type of the widget. It can be `checkbox`, `filepicker`, `text`, `combo`, `slider`.

If the type is filepicker, the setting name will be used in `HydraCore::loadFile()` once right before any rom is loaded.
So for example if you create a filepicker with name `bios_file` and the user configures it to the path `/home/user/mybios.bin`, every time a game is loaded `HydraCore::loadFile("bios_file", "/home/user/mybios.bin")` will be called.
Then the core developer can decide whether to do something with that path or not.

`extensions` is only used by the filepicker to specify a comma separated list of allowed extensions.

`required` is whether or not this field must not be left blank. Hydra won't allow users to load games otherwise.

`default` is the default value of the setting. It can be a boolean (true/false) for checkboxes or string for filepickers/text inputs.

`text_type` is the allowed type of text ie the charset. Leave empty to allow any character, or `num` for numeric, `hex` for hexadecimal or `pass` for password (hides the text in the gui)

`combo_options` is a comma separated list of options for `combo` type

`slider_min` is the minimum `slider` range
`slider_max` is the maximum `slider` range