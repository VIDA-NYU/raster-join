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

$scalabilityFolder = $OutputFolder + "\scalability"
if(!(Test-Path -Path $scalabilityFolder )){
    New-Item -ItemType directory -Path $scalabilityFolder
}

$inMemTimes = 1293838000, 1283328000, 1272808000, 1262298000, 1251178000, 1241268000
ForEach($endTime in $inMemTimes) {
	$opFile = $scalabilityFolder + "\taxi-in-memory.txt"
	$arguments = "--nIter", 6, "--joinType", "hybrid", "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", $endTime, "--inmem", "--outputTime", "$opFile", "--gpuMem", $GpuMem
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


