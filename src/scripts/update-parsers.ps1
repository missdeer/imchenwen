# Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
#
# This program is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see http://www.gnu.org/licenses/.


# Define functions to check version
function Get-Latest-Version-Github {
    param (
        $repo
    )
    $url = "https://api.github.com/repos/$repo/releases/latest"
    try {
        $response = Invoke-WebRequest $url -ErrorAction Stop
        return (ConvertFrom-Json -InputObject $response).tag_name
    }
    catch {
        Write-Output "Cannot get the latest version."
        Exit
    }
}

function Get-Current-Version {
    param (
        $plugin_name
    )
    $path = "$env:LOCALAPPDATA\imchenwen\version-$plugin_name.txt"
    if (Test-Path $path) {
        return Get-Content -Path "$path"
    } else {
        return "Not installed"
    }
}

function Save-Version-Info {
    param (
        $plugin_name,
        $version
    )
    $version > "$env:LOCALAPPDATA\imchenwen\version-$plugin_name.txt"
}

### Set Github Mirror
$github_mirror = "https://github.com"

### Update lux
Write-Output "-------- Checking lux's updates -------"

# Get current lux version
$current_version = Get-Current-Version "lux"
Write-Output "Current version: $current_version"

# Get latest lux version
$latest_version = Get-Latest-Version-Github "iawia002/lux"
Write-Output "Latest version: $latest_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "Lux is already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating lux -------------"

    # Download
    Write-Output "Downloading latest version..."
    $version_no_v = $latest_version.Substring(1)
    $url = "$github_mirror/iawia002/lux/releases/download/$latest_version/lux_${version_no_v}_Windows_x86_64.zip"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\lux.zip"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)

    # Extract
    Write-Output "Extracting lux..."
    Expand-Archive "$output" -DestinationPath "$env:LOCALAPPDATA\imchenwen" -Force
    Save-Version-Info "lux" $latest_version
}


### Update yt-dlp
Write-Output "-------- Checking yt-dlp's updates -------"

# Get latest yt-dlp version
$latest_version = Get-Latest-Version-Github "yt-dlp/yt-dlp"
Write-Output "Latest version: $latest_version"

# Get current yt-dlp version
$current_version = Get-Current-Version "yt-dlp"
Write-Output "Current version: $current_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "Yt-dlp already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating yt-dlp -------------"
    Write-Output "Downloading latest version..."
    $url = "$github_mirror/yt-dlp/yt-dlp/releases/download/$latest_version/yt-dlp.exe"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\yt-dlp.exe"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Save-Version-Info "yt-dlp" $latest_version
}


### Update ytdl-patched
Write-Output "-------- Checking ytdl-patched's updates -------"

# Get latest ytdl-patched version
$latest_version = Get-Latest-Version-Github "ytdl-patched/ytdl-patched"
Write-Output "Latest version: $latest_version"

# Get current ytdl-patched version
$current_version = Get-Current-Version "ytdl-patched"
Write-Output "Current version: $current_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "ytdl-patched already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating ytdl-patched -------------"
    Write-Output "Downloading latest version..."
    $url = "$github_mirror/ytdl-patched/ytdl-patched/releases/download/$latest_version/ytdl-patched-white.exe"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\ytdl-patched.exe"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Save-Version-Info "ytdl-patched" $latest_version
}


### Update ykdl
Write-Output "-------- Checking ykdl's updates -------"

# Get latest ykdl version
$latest_version = Get-Latest-Version-Github "missdeer/daily-weekly-build"
Write-Output "Latest version: $latest_version"

# Get current ykdl version
$current_version = Get-Current-Version "ykdl"
Write-Output "Current version: $current_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "ykdl already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating ykdl -------------"
    Write-Output "Downloading latest version..."
    $url = "$github_mirror/missdeer/daily-weekly-build/releases/download/$latest_version/ykdl.exe"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\ykdl.exe"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Save-Version-Info "ykdl" $latest_version
}


### Update you-get
Write-Output "-------- Checking you-get's updates -------"

# Get latest you-get version
$latest_version = Get-Latest-Version-Github "missdeer/daily-weekly-build"
Write-Output "Latest version: $latest_version"

# Get current you-get version
$current_version = Get-Current-Version "you-get"
Write-Output "Current version: $current_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "you-get already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating you-get -------------"
    Write-Output "Downloading latest version..."
    $url = "$github_mirror/missdeer/daily-weekly-build/releases/download/$latest_version/you-get.exe"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\you-get.exe"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Save-Version-Info "you-get" $latest_version
}


### Update youtube-dl
Write-Output "-------- Checking youtube-dl's updates -------"

# Get latest youtube-dl version
$latest_version = Get-Latest-Version-Github "missdeer/daily-weekly-build"
Write-Output "Latest version: $latest_version"

# Get current youtube-dl version
$current_version = Get-Current-Version "youtube-dl"
Write-Output "Current version: $current_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "youtube-dl already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating youtube-dl -------------"
    Write-Output "Downloading latest version..."
    $url = "$github_mirror/missdeer/daily-weekly-build/releases/download/$latest_version/youtube-dl.exe"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\youtube-dl.exe"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Save-Version-Info "youtube-dl" $latest_version
}



### Update plugins
Write-Output ""
Write-Output "---------- Checking plugins' updates ---------"

# Get current plugins version
$current_version = Get-Current-Version "plugins"
Write-Output "Current version: $current_version"

# Get latest plugins version
$latest_version = Get-Latest-Version-Github "coslyk/imchenwen-plugins"
Write-Output "Latest version: $latest_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "Plugins are already up-to-date."
} else {
    Write-Output "-------------- Updating plugins --------------"

    # Download
    Write-Output "Downloading plugins..."
    $url = "$github_mirror/coslyk/imchenwen-plugins/releases/download/$latest_version/plugins.zip"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\imchenwen\plugins.zip"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)

    # Extract
    Write-Output "Extracting plugins..."
    Expand-Archive "$output" -DestinationPath "$env:LOCALAPPDATA\imchenwen\plugins" -Force
    Save-Version-Info "plugins" $latest_version
    Write-Output "Finished. You need to restart imchenwen to load plugins."
}
