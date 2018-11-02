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
$polyList = $DataFolder + "\polys\nyc-polygons.txt"

$polySizes = 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536

ForEach($polySize in $polySizes) {
	$scalabilityFolder = $OutputFolder + "\scalability"
	if(!(Test-Path -Path $scalabilityFolder )){
	    New-Item -ItemType directory -Path $scalabilityFolder
	}
	$opFile = $scalabilityFolder + "\taxi-ooc-polygons.txt"
	$arguments = "--nIter", 6, "--joinType", "raster", "--accuracy", 10, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "$polySize", "--startTime", 1230768000, "--endTime", 1341128000, "--outputTime", "$opFile"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


