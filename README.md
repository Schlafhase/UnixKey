# UnixKey

UnixKey is a project that aims to make inputting arbitrary Unicode characters
using an English keyboard layout easier. This is especially useful if English is
not your native language. Take German for example: German words often contain ä,
ü, ö or ß. UnixKey allows you to set up rules so that you just type ae, ue, oe
or ss' which will then automatically get converted to the corresponding German
letters. With a few more rules, you can even exclude common English-only
segments (like "que") so you can still type most English words too. Obviously,
UnixKey also let's you undo a replacement by pressing a key you can configure.

## Quick Start

> [!WARNING]
>
> Unless you trust me completely for some reason, please make absolutely sure to
> **check the content** of `install.sh` before running the command below.

You can run the installer using
`sh -c "$(curl -fsSL https://github.com/Schlafhase/UnixKey/raw/refs/heads/master/install.sh)"`.
It will ask you for your `sudo` password because it has to move files to
/usr/lib/fcitx5/ and /usr/share/fcitx5/ which the user shouldn't have permission
to do.

You can also install UnixKey manually for more control.

### Manual Installation

**Dependencies**: fcitx5, cmake, ninja or make, libicu, C++ compiler

Start by installing UnixKey:

> [!NOTE]
>
> If you know what you're doing, you can replace /usr with another install
> prefix. In most cases, this isn't necessary and will break the installation.

```sh
git clone https://github.com/Schlafhase/UnixKey .
cd UnixKey
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

Now the last thing you need is a valid configuration. You can start by copying
the example configuration from src/unixkey.json to ~/.config/unixkey.json

You don't need the source any more. Delete it if you want:

```sh
cd ..
rm -rf UnixKey
```

Now restart fcitx5 using `fcitx5 -rd` (will keep running even after closing the
terminal) and finally configure fcitx5 using `fcitx5-configtool` to use the
UnixKey input method:

1. Open the config tool
2. Search for "UnixKey" in the "Search Input Method" field.
3. Double click on UnixKey to move it to your input methods.
4. (Optional) Move it to the top to make it the default.

### Uninstalling

To uninstall UnixKey, just remove the installed files

```sh
sudo rm /usr/share/fcitx5/addon/unixkey.conf
sudo rm /usr/share/fcitx5/inputmethod/unixkey.conf
sudo rm /usr/lib/fcitx5/unixkey.so
# optionally remove the config
rm ~/.config/unixkey.json
```

## Usage and configuration

The way you use UnixKey really depends on your configuration. But before going
more in depth about configuring, I'll explain basic usage.

The configuration lives in ~/.config/unixkey.json.

The concept is simple: type text, if it matches a rule, it gets replaced with
something else. A very useful feature that you might not immediately understand
is undoing replacements. The configuration specifies a key (`"undo_key"` in
`unixkey.json`) that undoes the last replacement, when pressed. The key is an
integer that represents the keycode.

> [!NOTE]
>
> Since the keycode isn't very user-friendly, I'll provide keycodes that I think
> make sense here:
>
> - **Backspace**: 65288
> - **Backslash**: 92
> - **Escape**: 65307
> - **F1-F35**: 65470 - 65504
> - **Other keys**: You can find a list of all keycodes in the
>   [fcitx5 source](https://github.com/fcitx/fcitx5/blob/master/src/lib/fcitx-utils/keysymgen.h).
>   The codes are in hex so you need to convert them to decimal before putting
>   them into your configuration.

You can also add modifiers to the key using the `"undo_modifiers"` property in
the configuration file. The value is a number. Below is a table of all
modifiers:

| Modifier                                         | Value      |
| ------------------------------------------------ | ---------- |
| No modifier                                      | 0          |
| Shift                                            | 1          |
| CapsLock                                         | 2          |
| Ctrl                                             | 4          |
| Alt                                              | 8          |
| NumLock                                          | 16         |
| Hyper                                            | 32         |
| Super                                            | 64         |
| Mod5                                             | 128        |
| MousePressed                                     | 256        |
| Meta                                             | 268435456  |
| Repeat (when the key was held and gets repeated) | 2147483648 |

> [!NOTE]
>
> You can combine multiple modifiers by adding their values together
> (technically you are performing a bitwise or). Common combinations are:
>
> - **Alt + Shift**: 8 + 1 = 9
> - **Ctrl + Shift**: 4 + 1 = 5
> - **Allow all modifiers**: 4294967295 (just all bits set to 1 on the 32-bit
>   integer)
> - and so on...

The second thing about undoing that can be configured is how long the last
replacement should be remembered (`"undo_reset"` in `unixkey.json`). A value of
5 means: After the replacement was made, 5 more insertions (usually single key
presses) can happen before the last replacement will be forgotten. You can set
this to a very high value to basically be able to undo whenever you like but I
think that low values make more sense here. This is because undoing a
replacement that happened potentially hundreds of characters before the cursor
is basically never intentional. Higher values will also make you unable to use
the specified key for anything else during the time, the last replacement is
remembered.

### Making an own configuration

Let's go through the process of making a configuration step by step:

I'll start by setting the undo settings:

```json
{
  "undo_key": 65307, // escape key (i know json doesn't have comments but i don't care)
  "undo_modifier": 1, // with shift modifier in case I want to use the escape key without undoing
  "undo_reset": 5
}
```

These two values were explained in detail already, so I'll just continue to the
actual replacement rules. First, we have the "case_sensitive" ruleset. As the
name suggests, these rules are **case sensitive** which means that they will
only trigger when the pattern is typed EXACTLY like in the config file.

For my German-letter configuration, this should include the umlauts because I
want to be able to type lowercase and uppercase variants:

```json
{
  "undo_key": 65307,
  "undo_modifier": 1,
  "undo_reset": 5,
  "case_sensitive": {
    "ae": "ä",
    "Ae": "Ä",
    "AE": "Ä",
    "oe": "ö",
    "Oe": "Ö",
    "OE": "Ö",
    "ue": "ü",
    "Ue": "Ü",
    "UE": "Ü"
  }
}
```

And you have probably guessed it, There is also a "case_insensitive" ruleset
which doesn't care about case. Let's put the ß in there because it doesn't have
a case. It's also great for little macros like [email] which can expand to your
email address.

```json
{
  "undo_key": 65307,
  "undo_modifier": 1,
  "undo_reset": 5,
  "case_sensitive": {
    "ae": "ä",
    "Ae": "Ä",
    "AE": "Ä",
    "oe": "ö",
    "Oe": "Ö",
    "OE": "Ö",
    "ue": "ü",
    "Ue": "Ü",
    "UE": "Ü"
  },
  "case_insensitive": {
    "ss'": "ß",
    "[email]": "me@example.com"
  }
}
```

This is a pretty good configuration already but theres one problem: Try to type
a common english word like "true", "value", "does" or similar and you will
notice that they turn to "trü", "valü" and "dös". This is, of course, not what
you want. UnixKey has a solution for this. You can set the replacement to
"UNIXKEY_PRESERVE" to tell UnixKey to leave the pattern alone when it matches.

```json
{
  "undo_key": 65307,
  "undo_modifier": 1,
  "undo_reset": 5,
  "case_sensitive": {
    "ae": "ä",
    "Ae": "Ä",
    "AE": "Ä",
    "oe": "ö",
    "Oe": "Ö",
    "OE": "Ö",
    "ue": "ü",
    "Ue": "Ü",
    "UE": "Ü"
  },
  "case_insensitive": {
    "ss'": "ß",
    "[email]": "me@example.com",
    "true": "UNIXKEY_PRESERVE",
    "blue": "UNIXKEY_PRESERVE",
    "value": "UNIXKEY_PRESERVE",
    "que": "UNIXKEY_PRESERVE", // in many cases even small segments like this one are enough (since "qü" never appears in german but "que" quite often in english)
    "queue": "UNIXKEY_PRESERVE",
    "does": "UNIXKEY_PRESERVE"
  }
}
```

You might think that you will have to put the whole english dictionary in there
but from my experience, even the small list of words to preserve in the config
above is enough to have a pretty consistent typing experience without unexpected
replacements (at least with English and German).

## How it works

