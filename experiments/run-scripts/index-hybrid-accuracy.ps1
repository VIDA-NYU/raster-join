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
$polyList = $DataFolder + "\polys\nyc_polys.txt"

$accFolder = $OutputFolder + "\accuracy"
if(!(Test-Path -Path $accFolder )){
    New-Item -ItemType directory -Path $accFolder
}
$opFile = $accFolder + "\taxi-acc-ooc.txt"

$indexArgs = "--nIter", 6, "--opAggregation", "$accFolder", "--joinType", "index", "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", 1341128000, "--outputTime", "$opFile", "--gpuMem", $GpuMem
$hybridArgs = "--nIter", 6, "--joinType", "hybrid", "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", 1341128000, "--outputTime", "$opFile", "--gpuMem", $GpuMem

Write-Host("executing $exe $indexArgs")
& "$exe" $indexArgs

Write-Host("executing $exe $hybridArgs")
& "$exe" $hybridArgs


