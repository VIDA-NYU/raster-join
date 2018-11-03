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

$bounds = 20, 10, 5, 3, 2, 1
ForEach($bound in $bounds) {
	$opFile = $accFolder + "\taxi-acc-ooc.txt"
	$arguments = "--nIter", 6, "--opAggregation", "$accFolder", "--joinType", "raster", "--accuracy", $bound, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", 1341128000, "--outputTime", "$opFile"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


