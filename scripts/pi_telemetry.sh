#!/usr/bin/env bash

# Raspberry Pi HardwareTelemetry Path Audit
# Read-only diagnostic script.
# Does not install packages or modify system configuration.

shopt -s nullglob

PASS=0
MISSING=0
INFO=0
WARN=0

section() {
    echo
    echo "============================================================"
    echo " $1"
    echo "============================================================"
}

check_file() {
    local path="$1"
    local label="$2"

    if [[ -f "$path" ]]; then
        if [[ -r "$path" ]]; then
            local value
            value=$(cat "$path" 2>/dev/null | tr '\0' ' ' | tr '\n' ' ')
            printf "[FOUND]   %-35s %s\n" "$label" "$path"
            printf "          VALUE: %s\n" "${value:-<empty>}"
            ((PASS++))
        else
            printf "[DENIED]  %-35s %s\n" "$label" "$path"
            ((WARN++))
        fi
    else
        printf "[MISSING] %-35s %s\n" "$label" "$path"
        ((MISSING++))
    fi
}

check_dir() {
    local path="$1"
    local label="$2"

    if [[ -d "$path" ]]; then
        printf "[FOUND]   %-35s %s\n" "$label" "$path"
        ((PASS++))
    else
        printf "[MISSING] %-35s %s\n" "$label" "$path"
        ((MISSING++))
    fi
}

check_command() {
    local cmd="$1"

    if command -v "$cmd" >/dev/null 2>&1; then
        printf "[FOUND]   Command available: %s\n" "$cmd"
        ((PASS++))
    else
        printf "[OPTIONAL] Command unavailable: %s\n" "$cmd"
        ((INFO++))
    fi
}

# ------------------------------------------------------------
section "1. SYSTEM INVENTORY"

check_file "/sys/firmware/devicetree/base/model" "Board model"
check_file "/sys/firmware/devicetree/base/serial-number" "Board serial"
check_file "/proc/cpuinfo" "CPU information"
check_file "/proc/meminfo" "System memory"
check_file "/proc/stat" "Boot time / btime"
check_file "/etc/hostname" "Hostname"

check_file "/sys/class/dmi/id/sys_vendor" "DMI manufacturer (alternative)"
check_file "/sys/class/dmi/id/product_name" "DMI model (alternative)"
check_file "/sys/class/dmi/id/product_uuid" "DMI system UUID (alternative)"
check_file "/sys/class/dmi/id/bios_version" "BIOS version (alternative)"

echo
echo "Boot time:"
grep '^btime' /proc/stat 2>/dev/null || true

echo
echo "Memory total:"
grep '^MemTotal' /proc/meminfo 2>/dev/null || true

# ------------------------------------------------------------
section "2. PROCESSOR INVENTORY"

check_dir "/sys/devices/system/cpu"

echo
echo "Discovered CPUs:"
for cpu in /sys/devices/system/cpu/cpu[0-9]*; do
    [[ -d "$cpu" ]] && echo "  $cpu"
done

echo
echo "CPU frequency paths:"
for cpu in /sys/devices/system/cpu/cpu[0-9]*; do
    [[ -d "$cpu" ]] || continue

    check_file "$cpu/cpufreq/cpuinfo_max_freq" "Maximum CPU frequency"
    check_file "$cpu/cpufreq/scaling_cur_freq" "Current CPU frequency"
done

echo
echo "Architecture:"
uname -m

echo
echo "CPU online:"
cat /sys/devices/system/cpu/online 2>/dev/null || echo "Unavailable"

# ------------------------------------------------------------
section "3. PROCESSOR THROTTLING"

check_command vcgencmd

if command -v vcgencmd >/dev/null 2>&1; then
    echo
    echo "Throttling flags:"
    vcgencmd get_throttled 2>&1

    echo
    echo "CPU temperature:"
    vcgencmd measure_temp 2>&1
fi

# ------------------------------------------------------------
section "4. MEMORY / ECC"

check_dir "/sys/devices/system/edac/mc"

for file in /sys/devices/system/edac/mc/mc*/*; do
    [[ -f "$file" ]] || continue
    check_file "$file" "EDAC memory counter"
done

echo
echo "Individual memory module information:"
if [[ -d /sys/devices/system/edac/mc ]]; then
    find /sys/devices/system/edac/mc -type f 2>/dev/null
else
    echo "EDAC directory unavailable."
fi

# ------------------------------------------------------------
section "5. STORAGE DISCOVERY"

check_command lsblk

if command -v lsblk >/dev/null 2>&1; then
    lsblk -o NAME,MODEL,SERIAL,SIZE,TRAN,ROTA
fi

check_dir "/sys/block"

for dev in /sys/block/*; do
    [[ -d "$dev" ]] || continue

    name=$(basename "$dev")

    # Ignore loop and ram devices for physical drive inventory.
    [[ "$name" == loop* || "$name" == ram* ]] && continue

    echo
    echo "---- DRIVE: $name ----"

    check_file "$dev/size" "Drive capacity (512-byte sectors)"
    check_file "$dev/queue/rotational" "Rotational indicator"
    check_file "$dev/device/model" "Drive model"
    check_file "$dev/device/vendor" "Drive manufacturer"
    check_file "$dev/device/serial" "Drive serial"
    check_file "$dev/device/rev" "Drive firmware revision"
    check_file "$dev/stat" "Drive I/O statistics"

    echo "Device details:"
    udevadm info --query=property --path="$dev" 2>/dev/null \
        | grep -E 'ID_MODEL=|ID_SERIAL=|ID_BUS=|ID_VENDOR=' || true
done

# ------------------------------------------------------------
section "6. STORAGE SMART / NVME"

check_command smartctl
check_command nvme

if command -v nvme >/dev/null 2>&1; then
    echo
    echo "NVMe devices:"
    nvme list 2>&1

    for dev in /dev/nvme[0-9]; do
        [[ -e "$dev" ]] || continue

        echo
        echo "---- NVME SMART: $dev ----"
        nvme smart-log "$dev" 2>&1
    done
fi

if command -v smartctl >/dev/null 2>&1; then
    echo
    echo "SMART device discovery:"
    smartctl --scan 2>&1
fi

# ------------------------------------------------------------
section "7. NETWORK INTERFACES"

check_dir "/sys/class/net"

for iface in /sys/class/net/*; do
    [[ -d "$iface" ]] || continue

    name=$(basename "$iface")

    echo
    echo "---- INTERFACE: $name ----"

    check_file "$iface/ifindex" "Interface index"
    check_file "$iface/address" "Current MAC address"
    check_file "$iface/operstate" "Link state"
    check_file "$iface/speed" "Link speed"
    check_file "$iface/duplex" "Duplex"
    check_file "$iface/type" "Interface type"

    check_dir "$iface/statistics"

    for stat in rx_bytes tx_bytes rx_errors tx_errors \
                rx_packets tx_packets rx_dropped tx_dropped \
                rx_crc_errors; do
        check_file "$iface/statistics/$stat" "$name $stat"
    done

    echo
    echo "Permanent MAC:"
    if command -v ethtool >/dev/null 2>&1; then
        ethtool -P "$name" 2>&1
    else
        echo "ethtool unavailable."
    fi
done

check_command ethtool

# ------------------------------------------------------------
section "8. THERMAL ZONES"

check_dir "/sys/class/thermal"

for zone in /sys/class/thermal/thermal_zone*; do
    [[ -d "$zone" ]] || continue

    echo
    echo "---- THERMAL ZONE: $(basename "$zone") ----"

    check_file "$zone/type" "Thermal sensor type"
    check_file "$zone/temp" "Thermal temperature"

    for threshold in "$zone"/*_trip_point_*_temp; do
        [[ -f "$threshold" ]] || continue
        check_file "$threshold" "Thermal trip threshold"
    done
done

# ------------------------------------------------------------
section "9. HARDWARE MONITORING SENSORS"

check_dir "/sys/class/hwmon"

for hwmon in /sys/class/hwmon/hwmon*; do
    [[ -d "$hwmon" ]] || continue

    echo
    echo "---- HWMON: $(basename "$hwmon") ----"

    check_file "$hwmon/name" "Hardware monitor name"

    for file in "$hwmon"/*_input; do
        [[ -f "$file" ]] || continue
        check_file "$file" "Sensor reading"
    done

    for file in "$hwmon"/*_label \
                "$hwmon"/*_max \
                "$hwmon"/*_crit \
                "$hwmon"/*_max_hyst; do
        [[ -f "$file" ]] || continue
        check_file "$file" "Sensor label / threshold"
    done
done

# ------------------------------------------------------------
section "10. RAS / ADDITIONAL HARDWARE SOURCES"

check_dir "/sys/devices/system/edac"
check_dir "/sys/firmware/devicetree/base"
check_dir "/sys/class/dmi/id"

echo
echo "Installed kernel:"
uname -a

echo
echo "Device tree model:"
tr '\0' '\n' < /sys/firmware/devicetree/base/model 2>/dev/null || true

# ------------------------------------------------------------
section "11. FINAL AUDIT SUMMARY"

echo "Found/readable checks : $PASS"
echo "Missing paths         : $MISSING"
echo "Permission warnings   : $WARN"
echo "Optional tools absent : $INFO"

echo
echo "============================================================"
echo " AUDIT COMPLETE"
echo "============================================================"

echo
echo "IMPORTANT:"
echo "- FOUND means the path exists and was readable."
echo "- MISSING means this particular candidate path is absent."
echo "- OPTIONAL means a utility is not installed."
echo "- Hardware-dependent properties may legitimately be absent."
echo "- Presence does not guarantee that a value has the required"
echo "  Redfish meaning."
echo "- Review sensor identities and drive/interface associations."
