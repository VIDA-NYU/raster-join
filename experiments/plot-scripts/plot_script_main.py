from __future__ import division
import math, os, struct, sys

from result_parser import readResults
from plot_templates import bar_plot, line_plot, box_plot, scatter_error_plot

import matplotlib, itertools, pylab
import matplotlib.pyplot as mplplot
import matplotlib.ticker
from matplotlib.dates import MonthLocator, DateFormatter
import numpy as np
import pandas as pd
import seaborn as sns

oneM = 1000000
oneB = 1000000000
oneK = 1000


#Point rendering + polygon rendering + poly memory
def get_processing_time(result):
    points_rendering = [result[7][i]/oneK for i in xrange(len(result[0])-1, -1, -1)]  
    poly_memory = [result[8][i]/oneK for i in xrange(len(result[0])-1, -1, -1)]
    poly_rendering = [result[9][i]/oneK for i in xrange(len(result[0])-1, -1, -1)]  
    return [x + y + z for x, y, z in zip(points_rendering, poly_memory, poly_rendering)]
    
#Triangulation + Poly Baseline Building
def get_pre_processing_time(result, divider):
    triangulation = [result[11][i]/divider for i in xrange(len(result[0])-1, -1, -1)]  
    poly_Baseline = [result[12][i]/divider for i in xrange(len(result[0])-1, -1, -1)]
    return [x + y for x, y in zip(triangulation, poly_Baseline)]

def get_triangulation_time(result):
    return [result[11][i] for i in xrange(len(result[11]))]  

def get_poly_indexing_time(result):
    return [result[12][i] for i in xrange(len(result[12]))] 

def get_pre_processing_time(result, divider):
    triangulation = [result[11][i]/divider for i in xrange(len(result[0])-1, -1, -1)]  
    poly_Baseline = [result[12][i]/divider for i in xrange(len(result[0])-1, -1, -1)]
    return [x + y for x, y in zip(triangulation, poly_Baseline)]

#Execute time minus everything else 
def get_point_memory_time(result):
    execute_time = [result[5][i]/oneK for i in xrange(len(result[0])-1, -1, -1)]
    processing_time = get_processing_time(result)
    pre_processing_time = get_pre_processing_time(result, oneK)
    return [x - y - z for x, y, z in zip(execute_time, processing_time, pre_processing_time)] 

#Execute time minus the pre-processing
def get_total_time(result):
    execute_time = [result[5][i]/oneK for i in xrange(len(result[0])-1, -1, -1)]
    pre_processing_time = get_pre_processing_time(result,oneK)
    return [x - y  for x, y in zip(execute_time, pre_processing_time)] 

def get_input_size_millions(result):
    return  [int(result[0][i]/oneM) for i in xrange(len(result[0])-1, -1, -1)]

def get_input_size_billions(result):
    return  [(result[0][i]/oneB) for i in xrange(len(result[0])-1, -1, -1)]

def get_number_of_polygons(result):
    return  [int(result[1][i]/oneK) for i in xrange(len(result[1]))]

def get_number_of_passes(result):
    return  [result[3][i] for i in xrange(len(result[0])-1, -1, -1)] 

def get_number_attrib(result):
    return  [result[4][i] for i in xrange(len(result[0]))]  

def get_resolution(result):
    return  [result[14][i] for i in xrange(len(result[0]))]  

def get_speedup(result_1, result_2):
    return [x/y for x, y in zip(result_1, result_2)]

def get_approaches(results):
    bounded = zip(*[r for r in results if r[2]==0])
    baseline = zip(*[r for r in results if r[2]==1])
    accurate = zip(*[r for r in results if r[2]==2])
    single_cpu_1024 = zip(*[r for r in results if r[2]==3 and r[3]==1024])
    multi_cpu_1024 = zip(*[r for r in results if r[2]==4 and r[3]==1024])
    single_cpu_4096 = zip(*[r for r in results if r[2]==3 and r[3]==4096])
    multi_cpu_4096 = zip(*[r for r in results if r[2]==4 and r[3]==4096]) 
    single_cpu_256 = zip(*[r for r in results if r[2]==3 and r[3]==256])
    multi_cpu_256 = zip(*[r for r in results if r[2]==4 and r[3]==256]) 
    return [bounded, baseline, accurate, single_cpu_1024, multi_cpu_1024, single_cpu_4096, multi_cpu_4096, single_cpu_256, multi_cpu_256]

def drop_indices(result, indices):
    return [i for j, i in enumerate(result) if j not in indices]

##############################

polygon_ids = []
accurate_results = []
approximate_results = []
c_min = []
c_max = []
p_min = []
p_max = []


c_min_norm = []
c_max_norm = []
p_min_norm = []
p_max_norm = []

band_width = []

def readAggregates(fn1, fn2):
    f1 = open(fn1, 'r')
    f2 = open(fn2, 'r')
    converter = (int, int)
    for line in f1:
        fields = line.strip('\n').split('\t')
        polygon_ids.append(int(fields[0]))
        accurate_results.append(int(fields[1]))
    for line in f2:
        fields = line.strip('\n').split('\t')
        approximate_results.append(int(fields[1]))

def readBounds(fn):
    f = open(fn, 'r') 
    for line in f:
        fields = line.strip('\n').split('\t')
        c_min.append(int(fields[1]))
        c_max.append(int(fields[2]))
        p_min.append(int(fields[3]))
        p_max.append(int(fields[4]))

def reset():
    del polygon_ids[:]
    del accurate_results[:]
    del approximate_results[:]
    del c_min[:]
    del c_max[:]
    del p_min[:]
    del p_max[:]
    del c_min_norm[:]
    del c_max_norm[:]
    del p_min_norm[:]
    del p_max_norm[:]
    del band_width[:]

def getAbsoluteError():
    return [abs(y-x) for x, y in zip(approximate_results, accurate_results)]


def getFilteredPercentRelativeError(): 
    selectors = np.logical_or([y >= 100 for y in accurate_results], np.logical_and([0 < y < 100 for y in accurate_results], [x > 25 for x in getAbsoluteError()])) #filtering 0s. 
    filtered_accurate_results = list(itertools.compress(accurate_results, selectors))
    filtered_approximate_results = list(itertools.compress(approximate_results, selectors))
    filtered_polygon_ids = list(itertools.compress(polygon_ids, selectors))
    return [abs((y-x)/y)*100 for x, y in zip(filtered_approximate_results, filtered_accurate_results)] 


if __name__=="__main__":

    if len(sys.argv) == 3:
        result_folder = str(sys.argv[1])
        exp = " ".join(sys.argv[2:]) 
    else:
        result_folder  = "../results-paper"
        exp = " ".join(sys.argv[1:]) 


    if exp == "figure_8" or exp == "all": 
        #Parse results
        results = readResults(result_folder+"/scalability/taxi-in-memory.txt")
        #Separate different approaches
        a = get_approaches(results)
        #Speedup 
        t =  get_processing_time(a[3])[1:] #choosing 1024 resolution

        measurements_list = [get_input_size_millions(a[0]), get_input_size_millions(a[2]), get_input_size_millions(a[1]), get_input_size_millions(a[4])]
        n = len(measurements_list[0])
        if all(len(x) == n for x in measurements_list):

            line_plot( 
                [get_speedup(t, get_processing_time(a[0])[1:]), get_speedup(t,
                get_processing_time(a[2])[1:]), get_speedup(t,
                get_processing_time(a[1])[1:]), get_speedup(t,
                get_processing_time(a[4])[1:])],
                get_input_size_millions(a[0])[1:],
                ['Bounded', 'Accurate', 'Baseline', 'Multi-CPU'],
                ['Input Size (millions of records)',
                'Speedup over single-core CPU'],
                ['maroon', 'grey', 'black', 'blue'],
                True,  # save the figure?
                '../figures-paper/8_1.png',
                'figure_8_1'
                )

            # Time plot
            line_plot(  
                [get_processing_time(a[0])[1:], get_processing_time(a[2])[1:],
                get_processing_time(a[1])[1:]],
                get_input_size_millions(a[0])[1:],
                ['Bounded', 'Accurate', 'Baseline'],
                ['Input Size (millions of records)', 'Time (sec)'],
                ['maroon', 'grey', 'black'],
                True, # save the figure?
                '../figures-paper/8_2.png',
                'figure_8_2'
                )
        else:
            print "Error in Figure 8 generation, the input file contains too many or too few measurements!"
    
    if exp == "figure_9" or exp == "all": 
        # Parse results
        results = readResults(result_folder+"/scalability/taxi-ooc.txt")
        # Separate different approaches
        a = get_approaches(results)
        # Speedup         
        t = get_total_time(a[3]) #choosing 1024 resolution

        measurements_list = [get_input_size_millions(a[0]), get_input_size_millions(a[2]), get_input_size_millions(a[1]), get_input_size_millions(a[4])]
        n = len(measurements_list[0])
        if all(len(x) == n for x in measurements_list):

            line_plot(  
                [get_speedup(t, get_total_time(a[0])), get_speedup(t,
                get_total_time(a[2])), get_speedup(t,
                get_total_time(a[1])), get_speedup(t,
                get_total_time(a[4]))],
                get_input_size_millions(a[0]),
                ['Bounded', 'Accurate', 'Baseline', 'Multi-CPU'],
                ['Input Size (millions of records)',
                'Speedup over single-core CPU'],
                ['maroon', 'grey', 'black', 'blue'],
                True, # save the figure?
                '../figures-paper/9_1.png',
                'figure_9_1'
                )

            # Split up
            bar_plot(  
                [[get_point_memory_time(a[0]), get_processing_time(a[0])],
                [get_point_memory_time(a[2]), get_processing_time(a[2])],
                [get_point_memory_time(a[1]), get_processing_time(a[1])]],
                get_input_size_millions(a[0]),
                [['Bounded (data transfer)', 'Bounded (processing)'],
                ['Accurate (data transfer)', 'Accurate (processing)'],
                ['Baseline (data transfer)', 'Baseline (processing)']],
                ['Input Size (millions of records)', 'Time (sec)'],
                ['maroon', 'grey', 'black'],
                False, # legend?
                '../figures-paper/9_2.png',
                True #stacked?
                )
        else:
            print "Error in Figure 9 generation, the input file contains too many or too few measurements!"

    
    if exp == "figure_10" or exp == "all":
        #Parse results
        results = readResults(result_folder+"/scalability/taxi-ooc-polygons.txt")
        #Separate different approaches
        a = get_approaches(results)

        measurements_list = [get_number_of_polygons(a[0]), get_number_of_polygons(a[2]), get_number_of_polygons(a[1]), get_number_of_polygons(a[4])]
        n = len(measurements_list[0])
        
        indices = [0,1,2,3]
        if not all(len(x) == n for x in measurements_list):
            indices_render = [0,1,2,3,8]
            indices_multicpu = [0,1,2,3,8]
        else:
            indices_render = indices
            indices_multicpu = indices

        # Removing some values
        processing_bounded = drop_indices(get_processing_time(a[0])[::-1], indices)
        processing_render = drop_indices(get_processing_time(a[1])[::-1], indices_render)
        processing_Accurate = drop_indices(get_processing_time(a[2])[::-1], indices)
        number_of_polygons = drop_indices(get_number_of_polygons(a[0]), indices)

        total_bounded = drop_indices(get_total_time(a[0])[::-1], indices)
        total_render = drop_indices(get_total_time(a[1])[::-1], indices_render)
        total_Accurate = drop_indices(get_total_time(a[2])[::-1], indices)
        total_multicpu = drop_indices(get_total_time(a[4])[::-1], indices_multicpu)

        bounded_preprocess = drop_indices(get_pre_processing_time(a[0], oneK)[::-1], indices)
        render_preprocess = drop_indices(get_pre_processing_time(a[1], oneK)[::-1], indices_render)
        Accurate_preprocess = drop_indices(get_pre_processing_time(a[2], oneK)[::-1], indices)
        multicpu_preprocess = drop_indices(get_pre_processing_time(a[4], oneK)[::-1], indices_multicpu)

        measurements_list = [number_of_polygons, total_bounded, total_render, total_Accurate, total_multicpu]
        n = len(measurements_list[0])

        if all(len(x) == n for x in measurements_list):
        

            # Processing time plot 
            line_plot(  
                [processing_bounded,
                processing_Accurate,
                processing_render],
                number_of_polygons,
                ['Bounded', 'Accurate', 'Baseline'],
                ['Number of polygons (in thousands)', 'Time (sec)'],
                ['maroon', 'grey', 'black'],
                True, # save the figure?
                '../figures-paper/10_3.png',
                'figure_10_3'
                )  


            # Total time plot 
            line_plot(  
                [total_bounded,
                total_Accurate,
                total_render,
                total_multicpu],
                number_of_polygons,
                ['Bounded', 'Accurate', 'Baseline', 'Multi-CPU'],
                ['Number of polygons (in thousands)', 'Time (sec) - logscale'],
                ['maroon', 'grey', 'black', 'blue'],
                True, # save the figure?
                '../figures-paper/10_2.png',
                'figure_10_2'
                )  
            
        
            # Pre-processing time plot 
            line_plot(  
                [bounded_preprocess,
                Accurate_preprocess,
                render_preprocess,
                multicpu_preprocess],
                number_of_polygons,
                ['Bounded', 'Accurate', 'Baseline', 'Multi-CPU'],
                ['Number of polygons (in thousands)', 'Time (sec) - logscale'],
                ['maroon', 'grey', 'black', 'blue'],
                True, # save the figure?
                '../figures-paper/10_1.png',
                'figure_10_1'
                )

        else:
            print "Error in Figure 10 generation, the input file contains too many or too few measurements!"

    if exp == "figure_11" or exp == "all":

        # Parse results
        results = readResults(result_folder+"/scalability/taxi-mem-attrib.txt")

        # Separate different approaches
        a = get_approaches(results)

        bar_plot(  
            [get_processing_time(a[0])[::-1],
             get_processing_time(a[2])[::-1],
             get_processing_time(a[1])[::-1]],
            get_number_attrib(a[0]),
            ['Bounded', 'Accurate', 'Baseline'],
            ['Number of filtered attributes', 'Time (sec)'],
            ['maroon', 'grey', 'black'],
            False, # legend?
            '../figures-paper/11_1.png',
            False #stacked?
            )
        
        # Parse results
        results = readResults(result_folder+"/scalability/taxi-ooc-attrib.txt")

        # Separate different approaches
        a = get_approaches(results)

        bar_plot(  
            [[get_point_memory_time(a[0])[::-1],
             get_processing_time(a[0])[::-1]],
             [get_point_memory_time(a[2])[::-1],
             get_processing_time(a[2])[::-1]],
             [get_point_memory_time(a[1])[::-1],
             get_processing_time(a[1])[::-1]]],
            get_number_attrib(a[0]),
            [['Bounded (data transfer)', 'Bounded (processing)'],
             ['Accurate (data transfer)', 'Accurate (processing)'],
             ['Baseline (data transfer)', 'Baseline (processing)']],
            ['Number of filtered attributes', 'Time (sec)'],
            ['maroon', 'grey', 'black'],
            False, # legend?
            '../figures-paper/11_2.png',
            True
            ) 


    if exp == "figure_12" or exp == "all":

        # Parse results
        results = readResults(result_folder+"/accuracy/taxi-acc-ooc.txt")

        # Separate different approaches
        a = get_approaches(results)

        resolution = get_resolution(a[0])

        Accurate_processing = get_processing_time(a[2])[::-1]*len(resolution)
        render_processing = get_processing_time(a[1])[::-1]*len(resolution)
        # Time plot
        line_plot(  
            [get_processing_time(a[0])[::-1], 
             Accurate_processing,
             render_processing],
            resolution,
            ['Bounded', 'Accurate', 'Baseline'],
            ['Error bound (in meters)', 'Time (sec)'],
            ['maroon', 'grey', 'black'],
            True, # save the figure?
            '../figures-paper/12_1.png',
            'figure_12_1'
            )

        readAggregates(result_folder+"/accuracy/raster_1341128000_0.csv",result_folder+"/accuracy/raster_1341128000_20.csv")
        
        if os.path.exists(result_folder+"/accuracy/err_bound_1341128000_20.csv"):
            readBounds(result_folder+"/accuracy/err_bound_1341128000_20.csv") 
            y_err_low = [((approximate_results[i] - p_min[i])/100000000) for i in xrange(len(approximate_results))]
            y_err_high = [((p_max[i] - approximate_results[i])/100000000) for i in xrange(len(approximate_results))]
            accurate_div = [(accurate_results[i]/100000000) for i in xrange(len(accurate_results))]
            approximate_div = [(approximate_results[i]/100000000) for i in xrange(len(approximate_results))]
            scatter_error_plot("../figures-paper/12_3", 0, y_err_low, y_err_high, accurate_div, approximate_div) 
            scatter_error_plot("../figures-paper/12_3_zoom.png", 1, y_err_low, y_err_high, accurate_div, approximate_div) 


        filtered_errors = []    
        filtered_errors.append(getFilteredPercentRelativeError())
        for i in [10, 5, 3, 2, 1]:
            reset()
            readAggregates(result_folder+"/accuracy/raster_1341128000_0.csv",result_folder+"/accuracy/raster_1341128000_" + str(i) + ".csv")
            filtered_errors.append(getFilteredPercentRelativeError())

        box_plot(filtered_errors[::-1], ['1', '2', '3', '5', '10', '20'], "../figures-paper/12_2.png", "nothing")


    if exp == "figure_13" or exp == "all" and os.path.isfile(result_folder+"/scalability/twitter-cpu-ooc.txt"): 

        #Parse results
        results = readResults(result_folder+"/scalability/twitter-cpu-ooc.txt")
        #Separate different approaches
        a = get_approaches(results)

        # Total time plot
        line_plot(  
            [get_total_time(a[0]), get_total_time(a[2]),
             get_total_time(a[1])],
            get_input_size_billions(a[0]),
            ['Bounded', 'Accurate', 'Baseline'],
            ['Input Size (billions of records)', 'Time (sec)'],
            ['maroon', 'grey', 'black'],
            True, # save the figure?
            '../figures-paper/13_1.png',
            'figure_13_1'
            )

        # Processing time plot  
        line_plot(  
            [get_processing_time(a[0]), get_processing_time(a[2]),
             get_processing_time(a[1])],
            get_input_size_billions(a[0]),
            ['Bounded', 'Accurate', 'Baseline'],
            ['Input Size (billions of records)', 'Time (sec)'],
            ['maroon', 'grey', 'black'],
            True, # save the figure?
            '../figures-paper/13_2.png',
            'figure_13_2'
            )
    
    if exp == "figure_14" or exp == "all" and os.path.isfile(result_folder+"/accuracy/twitter-acc.txt"):
        results = readResults(result_folder+"/accuracy/twitter-acc.txt")

        # Separate different approaches
        a = get_approaches(results)

        resolution = get_resolution(a[0])

        Accurate_processing = get_processing_time(a[2])[::-1]*len(resolution)
        render_processing = get_processing_time(a[1])[::-1]*len(resolution)
        # Time plot
        line_plot(  
            [get_processing_time(a[0])[::-1], 
             Accurate_processing,
             render_processing],
            resolution,
            ['Bounded', 'Accurate', 'Baseline'],
            ['Error bound (in meters)', 'Time (sec)'],
            ['maroon', 'grey', 'black'],
            True, # save the figure?
            '../figures-paper/14_1.png',
            'figure_14_1'
            )

        readAggregates(result_folder+"/accuracy/raster_1412073963_0.csv",result_folder+"/accuracy/raster_1412073963_2000.csv")
        
        filtered_errors = []    
        filtered_errors.append(getFilteredPercentRelativeError())
        for i in [1000, 500, 250, 100]:
            reset()
            readAggregates(result_folder+"/accuracy/raster_1412073963_0.csv",result_folder+"/accuracy/raster_1412073963_" + str(i) + ".csv")
            filtered_errors.append(getFilteredPercentRelativeError())
        
        box_plot(filtered_errors[::-1], ['100', '250', '500', '1000', '2000'], "../figures-paper/14_2.png","twitter")

