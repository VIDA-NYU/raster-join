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

$accFolder = $OutputFolder + "\accuracy"
if(!(Test-Path -Path $accFolder )){
    New-Item -ItemType directory -Path $accFolder
}

$opFile = $accFolder + "\taxi-acc-ooc.txt"
$arguments = "--nIter", 1, "--opAggregation", "$accFolder", "--joinType", "errorbounds", "--accuracy", 20, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", 1341128000
Write-Host("executing $exe $arguments")
& "$exe" $arguments


