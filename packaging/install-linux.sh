#!/usr/bin/env bash

set -Eeuo pipefail

strata_script_dir=$(cd -P "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)
strata_root=$(cd "$strata_script_dir/.." >/dev/null 2>&1 && pwd -P)
strata_launcher=$strata_root/bin/ae1200-strata
strata_template=$strata_script_dir/ae1200-strata.desktop.in
strata_icon_source=$strata_root/assets/ae1200-strata.svg
strata_marker='X-AE1200-Strata-Managed=true'
strata_icon_marker='Managed by the AE1200 Strata Linux installer'
strata_dry_run=false
strata_desktop_temp=
strata_icon_temp=

strata_usage() {
    cat <<'EOF'
Usage: packaging/install-linux.sh [--dry-run]

Install an AE1200 Strata application-menu entry and icon for the current user.
The menu entry launches this repository in place; no project files are copied.

  --dry-run  Show target paths without writing files
  -h, --help Show this help
EOF
}

strata_die() {
    printf 'AE1200 Strata installer: %s\n' "$1" >&2
    exit 1
}

strata_cleanup() {
    [[ -z "$strata_desktop_temp" || ! -e "$strata_desktop_temp" ]] || rm -f -- "$strata_desktop_temp"
    [[ -z "$strata_icon_temp" || ! -e "$strata_icon_temp" ]] || rm -f -- "$strata_icon_temp"
}
trap strata_cleanup EXIT

while (($#)); do
    case $1 in
        --dry-run)
            strata_dry_run=true
            ;;
        -h|--help)
            strata_usage
            exit 0
            ;;
        *)
            strata_usage >&2
            strata_die "unknown option: $1"
            ;;
    esac
    shift
done

if [[ -n "${XDG_DATA_HOME:-}" ]]; then
    [[ "$XDG_DATA_HOME" == /* ]] || strata_die 'XDG_DATA_HOME must be an absolute path'
    strata_data_home=$XDG_DATA_HOME
elif [[ -n "${HOME:-}" ]]; then
    [[ "$HOME" == /* ]] || strata_die 'HOME must be an absolute path'
    strata_data_home=$HOME/.local/share
else
    strata_die 'neither XDG_DATA_HOME nor HOME is set'
fi

strata_applications_dir=$strata_data_home/applications
strata_icons_dir=$strata_data_home/icons/hicolor/scalable/apps
strata_desktop_target=$strata_applications_dir/ae1200-strata.desktop
strata_icon_target=$strata_icons_dir/ae1200-strata.svg

[[ -x "$strata_launcher" ]] || strata_die "launcher is missing or not executable: $strata_launcher"
[[ -f "$strata_template" ]] || strata_die "desktop template is missing: $strata_template"
[[ -f "$strata_icon_source" ]] || strata_die "application icon is missing: $strata_icon_source"

strata_check_target() {
    local strata_target=$1
    local strata_expected_marker=$2

    if [[ -e "$strata_target" || -L "$strata_target" ]]; then
        [[ ! -L "$strata_target" && -f "$strata_target" ]] || \
            strata_die "refusing to replace non-regular target: $strata_target"
        grep -Fq "$strata_expected_marker" "$strata_target" || \
            strata_die "refusing to replace an unmanaged file: $strata_target"
    fi
}

strata_check_target "$strata_desktop_target" "$strata_marker"
strata_check_target "$strata_icon_target" "$strata_icon_marker"

printf 'Application entry: %s\n' "$strata_desktop_target"
printf 'Application icon:  %s\n' "$strata_icon_target"
printf 'Repository:        %s\n' "$strata_root"

if [[ "$strata_dry_run" == true ]]; then
    printf 'Dry run complete; no files were written.\n'
    exit 0
fi

mkdir -p "$strata_applications_dir" "$strata_icons_dir"

# Desktop Entry command arguments have their own quoting rules. Escape the four
# characters that are special inside a double-quoted Exec argument.
strata_exec_escaped=${strata_launcher//\\/\\\\}
strata_exec_escaped=${strata_exec_escaped//\"/\\\"}
strata_exec_escaped=${strata_exec_escaped//\`/\\\`}
strata_exec_escaped=${strata_exec_escaped//\$/\\\$}

strata_desktop_temp=$(mktemp "$strata_applications_dir/.ae1200-strata.desktop.XXXXXX")
while IFS= read -r strata_line || [[ -n "$strata_line" ]]; do
    if [[ "$strata_line" == 'Exec=@EXEC@' ]]; then
        printf 'Exec="%s"\n' "$strata_exec_escaped"
    else
        printf '%s\n' "$strata_line"
    fi
done <"$strata_template" >"$strata_desktop_temp"
chmod 0644 "$strata_desktop_temp"
mv -f -- "$strata_desktop_temp" "$strata_desktop_target"
strata_desktop_temp=

strata_icon_temp=$(mktemp "$strata_icons_dir/.ae1200-strata.svg.XXXXXX")
install -m 0644 "$strata_icon_source" "$strata_icon_temp"
mv -f -- "$strata_icon_temp" "$strata_icon_target"
strata_icon_temp=

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$strata_applications_dir" >/dev/null 2>&1 || true
fi

printf 'Installed. Open “AE1200 Emulator” from the application menu.\n'
