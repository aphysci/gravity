
# Elevate to admin - https://stackoverflow.com/questions/7690994/powershell-running-a-command-as-administrator
If (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))
{
    # The original script is modified to pass through the command-line parameter
    $arguments = "& '" + $myinvocation.mycommand.definition + "'"
    Start-Process powershell -Verb runAs -ArgumentList $arguments
    Break
}

# Base location for the registry keys we need to add
$DevConfigRegBase = "HKLM:\SOFTWARE\Kitware\CMake\Config"

Set-ItemProperty -path $DevConfigRegBase -name FilesystemRetryCount -value 48
Set-ItemProperty -path $DevConfigRegBase -name FilesystemRetryDelay -value 4096

