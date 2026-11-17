#!/bin/sh
set -eu

if [ "$#" -ne 1 ] || [ ! -d "$1" ]; then
    echo "Expected the macOS test app bundle as the only argument." >&2
    exit 1
fi

app_bundle=$1
warn() {
    printf 'WARNING: %s\n' "$1" >&2
}

macos_dir=$app_bundle/Contents/MacOS
resources_dir=$app_bundle/Contents/Resources
/bin/mkdir -p "$resources_dir"
for resource_name in Font Shader; do
    source_dir=$macos_dir/$resource_name
    target_dir=$resources_dir/$resource_name
    if [ ! -d "$source_dir" ] || [ -L "$source_dir" ]; then
        echo "Expected copied KVS resources at $source_dir." >&2
        exit 1
    fi
    /bin/rm -rf "$target_dir"
    /bin/mv "$source_dir" "$target_dir"
    /bin/ln -s "../Resources/$resource_name" "$source_dir"
done

legacy_config=$macos_dir/config.ini
if [ -f "$legacy_config" ] && [ ! -L "$legacy_config" ]; then
    /bin/mv -f "$legacy_config" "$resources_dir/config.ini"
fi

identity=
if identity_output=$(/usr/bin/security find-identity -v -p codesigning); then
    identities=$(printf '%s\n' "$identity_output" |
        /usr/bin/sed -nE '/"Apple Development:/s/^[[:space:]]*[0-9]+\) ([[:xdigit:]]{40}) .*/\1/p')
    requested_identity=${PBVR_TEST_CODESIGN_IDENTITY:-}

    if [ -n "$requested_identity" ]; then
        if printf '%s\n' "$identities" | /usr/bin/grep -Fxq "$requested_identity"; then
            identity=$requested_identity
        else
            warn "PBVR_TEST_CODESIGN_IDENTITY is not a valid Apple Development identity hash; trying ad hoc signing."
        fi
    else
        identity_count=$(printf '%s\n' "$identities" | /usr/bin/sed '/^$/d' | /usr/bin/wc -l | /usr/bin/tr -d '[:space:]')
        case "$identity_count" in
            1) identity=$identities ;;
            0) warn "No valid Apple Development signing identity found; trying ad hoc signing." ;;
            *) warn "Found $identity_count valid Apple Development signing identities; set PBVR_TEST_CODESIGN_IDENTITY to select one. Trying ad hoc signing." ;;
        esac
    fi
else
    warn "Could not list code signing identities; trying ad hoc signing."
fi

if [ -n "$identity" ]; then
    echo "Signing test app with Apple Development identity."
    if /usr/bin/codesign --force --sign "$identity" "$app_bundle" &&
        /usr/bin/codesign --verify --verbose=2 "$app_bundle"; then
        exit 0
    fi
    warn "Apple Development signing or verification failed; trying ad hoc signing."
fi

echo "Signing test app ad hoc."
if /usr/bin/codesign --force --sign - "$app_bundle" &&
    /usr/bin/codesign --verify --verbose=2 "$app_bundle"; then
    exit 0
fi
warn "Ad hoc signing or verification failed. The build will continue, but the test app may not launch."
exit 0
