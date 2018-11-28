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

$attribs = 0,1,2,3
ForEach($nAttrib in $attribs) {
	$opFile = $scalabilityFolder + "\taxi-ooc-attrib.txt"
	$arguments = "--nIter", 6, "--joinType", "index", "--indexRes", 1024, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", 1272808000, "--nAttrib", $nAttrib, "--outputTime", "$opFile", "--gpuMem", $GpuMem
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


