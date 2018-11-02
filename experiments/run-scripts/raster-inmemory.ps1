param
(
	[Parameter(Mandatory)]
	[string]$ProgramDir,
	[Parameter(Mandatory)]
	[string]$DataFolder,
	[Parameter(Mandatory)]
	[string]$OutputFolder
)

$exe = $ProgramDir + "\RasterJoin.exe"
$index = $DataFolder + "\taxi\taxi_full_index"
$polyList = $DataFolder + "\polys\nyc_polys.txt"

$inMemTimes = 1293838000, 1283328000, 1272808000, 1262298000, 1251178000, 1241268000

ForEach($endTime in $inMemTimes) {
	$scalabilityFolder = $OutputFolder + "\scalability"
	if(!(Test-Path -Path $scalabilityFolder )){
	    New-Item -ItemType directory -Path $scalabilityFolder
	}
	$opFile = $scalabilityFolder + "\taxi-in-memory.txt"
	$arguments = "--nIter", 6, "--joinType", "raster", "--accuracy", 10, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", $endTime, "--inmem", "--outputTime", "$opFile"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


