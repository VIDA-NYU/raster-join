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

$attribs = 0,1,2,3

ForEach($nAttrib in $attribs) {
	$scalabilityFolder = $OutputFolder + "\scalability"
	if(!(Test-Path -Path $scalabilityFolder )){
	    New-Item -ItemType directory -Path $scalabilityFolder
	}
	$opFile = $scalabilityFolder + "\taxi-ooc-attrib.txt"
	$arguments = "--nIter", 6, "--joinType", "raster", "--accuracy", 10, "--backendIndexName", "$index", "--locAttrib", 1, "--polygonList", "$polyList", "--polygonDataset", "neigh", "--startTime", 1230768000, "--endTime", 1272808000, "--nAttrib", $nAttrib, "--outputTime", "$opFile"
	Write-Host("executing $exe $arguments")
	& "$exe" $arguments
}


