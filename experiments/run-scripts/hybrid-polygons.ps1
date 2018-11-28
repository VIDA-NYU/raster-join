param
(
	[Parameter(Mandatory)]
	[string]$ProgramDir,
	[Parameter(Mandatory)]
	[string]$DataFolder,
	[Parameter(Mandatory)]
	[string]$OutputFolder,
	[int]$GpuMem=3072
)

$exe = $ProgramDir + "\RasterJoin.exe"
$index = $DataFolder + "\taxi\taxi_full_index"
$polyList = $DataFolder + "\polys\nyc-polygons.txt"

$scalabilityFolder = $OutputFolder + "\scalability"
if(!(Test-Path -Path $scalabilityFolder )){
    New-Item -ItemType directory -Path $scalabilityFolder
}

$polySizes = 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536
ForEach($polySize in $polySizes) {
	$opFile = $scalabilityFolder + "\taxi-ooc-polygons.txt"
	$arguments = "--nIter", 6, "--joinType", "hybrid", "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "$polySize", "--startTime", 1230768000, "--endTime", 1341128000, "--outputTime", "$opFile", "--gpuMem", $GpuMem
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


