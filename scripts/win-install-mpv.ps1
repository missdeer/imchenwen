# 定义目标URL
$url = "https://sourceforge.net/projects/mpv-player-windows/files/libmpv/"

# 使用Invoke-WebRequest获取页面内容
$response = Invoke-WebRequest -Uri $url

# 使用正则表达式匹配下载链接
$pattern = 'mpv-dev-x86_64-v3-[0-9]{8}-git-[0-9a-f]{7,9}\.7z'
$matches = Select-String -Pattern $pattern -InputObject $response.Content -AllMatches
$downloadLink = $matches.Matches | Select-Object -First 1 -ExpandProperty Value

# 构造完整的下载URL
$baseUri = "https://versaweb.dl.sourceforge.net/project/mpv-player-windows/libmpv/"
$fullUrl = $baseUri + $downloadLink.TrimStart('/')

# 下载文件
$downloadPath = "..\libmpv"
$localFilePath = $downloadPath + "\" + $downloadLink
			   
# 打印完整的下载URL
Write-Host "下载 $fullUrl 到 $localFilePath"

# 执行下载并打印结果
$result = Invoke-WebRequest -Uri $fullUrl -OutFile $localFilePath
Write-Host "下载结果: $result"

# 使用7z解压文件
& "C:\Program Files\7-Zip\7z.exe" x $localFilePath -o"$downloadPath" -y

Remove-Item $localFilePath
