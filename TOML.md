## Hydra core specific settings
Hydra cores use toml format to generate widgets that appear in the GUI at runtime.

Each widget is a toml table like so:
```toml
[my_setting_name]
name = "My important BIOS file!"
description = "We need this BIOS file please provide it!"
category = "BIOS files"
type = "filepicker"
required = true
```

Hydra then uses these widgets to store core specific settings in a settings.json

`my_setting_name` is for hydra to identify this setting. The actual name becomes <CoreName>_my_setting_name
where <CoreName> is the current core name.

`name` is the pretty name of the setting that appears in the GUI.

`description` is a description that appears when you hover the widget