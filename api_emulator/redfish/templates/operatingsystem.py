from copy import deepcopy

OPERATING_SYSTEM_TEMPLATE = {
    "@odata.id": "{rb}{suffix}/{suffix_id}/OperatingSystem",
    "@odata.type": "#OperatingSystem.v1_1_0.OperatingSystem",
    "Name": "Computer Operating System",
    "Id": "OperatingSystem",
    "Type": "{type}",
    "Description": "Operating System of the current computer system",
    "Kernel": {
        "Name": "{kernel_type}",
        "Version": "{kernel_ver}"
    },
    "Status": {
        "State": "Enabled",
        "Health": "OK"
    },
}

def format_operating_system_template(**kwargs):
    """
    Formats the operating system template -- returns the template
    """
    # default params
    defaults = {
        'rb': '/redfish/v1/',
        'suffix': 'Systems',
        'type': 'Linux',
        'kernel_type': 'Linux Kernel'
    }
    defaults.update(kwargs)

    c = deepcopy(OPERATING_SYSTEM_TEMPLATE)
    c['@odata.id'] = c['@odata.id'].format(**defaults)
    c['Type'] = c['Type'].format(**defaults)
    c['Kernel']['Name'] = c['Kernel']['Name'].format(**defaults)
    c['Kernal']['Version'] = c['Kernel']['Version'].format(**defaults)

    return c 