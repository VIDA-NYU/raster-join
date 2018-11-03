param
(
	[Parameter(Mandatory)]
	[string]$ProgramDir,
	[Parameter(Mandatory)]
	[string]$DataFolder,
	[Parameter(Mandatory)]
	[string]$OutputFolder
)

$exe = $ProgramDir + "\CPUJoin.exe"
$index = $DataFolder + "\taxi\taxi_full_index"
$polyList = $DataFolder + "\polys\nyc_polys.txt"

$scalabilityFolder = $OutputFolder + "\scalability"
if(!(Test-Path -Path $scalabilityFolder )){
    New-Item -ItemType directory -Path $scalabilityFolder
}
$polyIndexFolder = $DataFolder + "\polys\index"
if(!(Test-Path -Path $polyIndexFolder )){
    New-Item -ItemType directory -Path $polyIndexFolder
}

$inMemTimes = 1293838000, 1283328000, 1272808000, 1262298000, 1251178000, 1241268000
ForEach($endTime in $inMemTimes) {
	$opFile = $scalabilityFolder + "\taxi-in-memory.txt"
	
	$arguments = "--nIter", 6, "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", $endTime, "--outputTime", "$opFile"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments

	$arguments = "--nIter", 6, "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", $endTime, "--outputTime", "$opFile", "--singleCore"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}

$oocTimes = 1388468000,1372668000,1356868000,1341128000,1325378000,1309608000
ForEach($endTime in $oocTimes) {
	$opFile = $scalabilityFolder + "\taxi-ooc.txt"
	
	$arguments = "--nIter", 6, "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", $endTime, "--outputTime", "$opFile"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments

	$arguments = "--nIter", 6, "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", $endTime, "--outputTime", "$opFile", "--singleCore"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}

