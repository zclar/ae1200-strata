#!/usr/bin/env bash

set -Eeuo pipefail

strata_marker='X-AE1200-Strata-Managed=true'
strata_icon_marker='Managed by the AE1200 Strata Linux installer'
strata_dry_run=false

strata_usage() {
    cat <<'EOF'
Usage: packaging/uninstall-linux.sh [--dry-run]

Remove the current user's AE1200 Strata application-menu entry and icon.
Repository files, builds, and simulator data are never removed.

  --dry-run  Show what would be removed without writing files
  -h, --help Show this help
EOF
}

strata_die() {
    printf 'AE1200 Strata uninstaller: %s\n' "$1" >&2
    exit 1
}

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
strata_desktop_target=$strata_applications_dir/ae1200-strata.desktop
strata_icon_target=$strata_data_home/icons/hicolor/scalable/apps/ae1200-strata.svg

strata_remove_managed_file() {
    local strata_target=$1
    local strata_expected_marker=$2

    if [[ ! -e "$strata_target" && ! -L "$strata_target" ]]; then
        printf 'Not installed: %s\n' "$strata_target"
        return
    fi
    [[ ! -L "$strata_target" && -f "$strata_target" ]] || \
        strata_die "refusing to remove non-regular target: $strata_target"
    grep -Fq "$strata_expected_marker" "$strata_target" || \
        strata_die "refusing to remove an unmanaged file: $strata_target"

    if [[ "$strata_dry_run" == true ]]; then
        printf 'Would remove: %s\n' "$strata_target"
    else
        rm -f -- "$strata_target"
        printf 'Removed: %s\n' "$strata_target"
    fi
}

strata_remove_managed_file "$strata_desktop_target" "$strata_marker"
strata_remove_managed_file "$strata_icon_target" "$strata_icon_marker"

if [[ "$strata_dry_run" == false ]] && command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$strata_applications_dir" >/dev/null 2>&1 || true
fi

if [[ "$strata_dry_run" == true ]]; then
    printf 'Dry run complete; no files were removed.\n'
else
    printf 'AE1200 Strata desktop integration was uninstalled.\n'
fi
