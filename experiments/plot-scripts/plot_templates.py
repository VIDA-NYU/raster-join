import matplotlib, itertools, pylab
import matplotlib.pyplot as mplplot
import matplotlib.ticker
from matplotlib.dates import MonthLocator, DateFormatter
from matplotlib.ticker import MaxNLocator
import numpy as np
import pandas as pd
import seaborn as sns


def bar_plot(
    approaches,
    inputUnit,
    approachLabels,
    axisLabels,
    colors,
    show,
    figName,
    stacked,
    ):
            
	fig = mplplot.figure(figsize=(12,9), dpi=200)
      	        
	with sns.axes_style("white"):
		sns.set_style("ticks")
		sns.set_context("talk")

		BAR_GROUP_WIDTH = 1.0
		BAR_GROUP_MARGIN = 0.5
		SEPERATE_BARS_PER_GROUP = 3
		SINGLE_BAR_MARGIN = 0.1

		SINGLE_BAR_WIDTH = (
		(BAR_GROUP_WIDTH - (SEPERATE_BARS_PER_GROUP - 1) * SINGLE_BAR_MARGIN)
		/ SEPERATE_BARS_PER_GROUP)
		assert SINGLE_BAR_WIDTH > 0, \
		'Not enough space for the bars, adjust the relevant constants'

		# plot details
		line_width = 1
		opacity = 0.7
		fontsize = 37

		group_step = BAR_GROUP_WIDTH + BAR_GROUP_MARGIN
		pos = []
        	#positions of the first bar in the group
        	pos.append(np.arange(start=0, stop=len(inputUnit) * group_step, step=group_step))

       		for _ in itertools.repeat(None, len(approaches)):
        		pos.append(pos[-1] + SINGLE_BAR_MARGIN + SINGLE_BAR_WIDTH)
        	
        	
        	if SEPERATE_BARS_PER_GROUP % 2:
            		middle_bar_idx = int(SEPERATE_BARS_PER_GROUP / 2)
            		bar_group_middle = pos[0] + (
                	middle_bar_idx * (SINGLE_BAR_MARGIN + SINGLE_BAR_WIDTH))
        	else:
            		bar_group_middle = pos[0] + BAR_GROUP_WIDTH / 2

        	# make bar plots
        	for i in xrange(len(approaches)):
			if(stacked):
        			mplplot.bar(pos[i], approaches[i][0], SINGLE_BAR_WIDTH,
                              		alpha=opacity,
                              		color='white',
                              		edgecolor=colors[i],
                              		linewidth=line_width,
                              		hatch='//',
                              		label=approachLabels[i][0])
        			mplplot.bar(pos[i], approaches[i][1], SINGLE_BAR_WIDTH,
                              		bottom=approaches[i][0],
                              		color=colors[i],
                              		edgecolor=colors[i],
                              		linewidth=line_width,
                              		label=approachLabels[i][1])
			else:
				mplplot.bar(pos[i], approaches[i], SINGLE_BAR_WIDTH,
                              		color=colors[i],
                              		edgecolor=colors[i],
                              		linewidth=line_width,
                              		label=approachLabels[i])
			
        	mplplot.xticks(bar_group_middle, inputUnit, fontsize=fontsize)
        	for tick_line in fig.get_axes()[0].get_xticklines():
            		tick_line.set_visible(False)
        	mplplot.xlabel(axisLabels[0], fontsize=fontsize, fontweight ='heavy')
        	
		mplplot.yticks(fontsize=fontsize)  
        #pylab.ylim(ymax=0.5)
        mplplot.tick_params(axis='y', which='major', pad=15)
        mplplot.ylabel(axisLabels[1], fontsize=fontsize, fontweight ='heavy')
        sns.despine()
        ax = fig.gca()
		# Make thicker the spines
        mplplot.setp(ax.spines.values(), linewidth=3)
		# Make thicker the ticks
        ax.xaxis.set_tick_params(width=3)
        ax.yaxis.set_tick_params(width=3)
      
        mplplot.legend(loc='best', fontsize=24)
        if show:
            pylab.legend(loc='center', bbox_to_anchor=(0.5, 1.1), ncol=3, fontsize=fontsize)
        	 	
	#mplplot.show()
	mplplot.tight_layout() 
      
	fig.savefig(figName, facecolor=fig.get_facecolor(), 
                    edgecolor='none', dpi=200) #bbox_extra_artists=(lgd,) bbox_inches='tight'
        	
def line_plot(
    approaches,
    inputUnit,
    approachLabels,
    axisLabels,
    colors,
    save,
    figName,
    specificSettings,
    ):

    fig = mplplot.figure(figsize=(12, 9), dpi=200)

    fontsize = 37

    with sns.axes_style("white"):
    	sns.set_style("ticks")
    	sns.set_context("talk")
    	plot = fig.add_subplot(111)
    	handles = []
    	legends = []
    	markers = [  # defining some markers
        	'o',
        	'^',
        	's',
        	'v',
        	'D',
        	'*',
        	]
    	

        if specificSettings == "figure_10_1" or specificSettings == "figure_10_2":
            for i in xrange(len(approaches)):
        	   (h, ) = plot.semilogy ( 
            	inputUnit,
            	approaches[i],
            	marker=markers[i],
            	markersize=18,
            	linewidth=7,
            	color=colors[i],
                )
        	   handles.append(h)
        	   legends.append(approachLabels[i])
        else:
            for i in xrange(len(approaches)):
                (h, ) = plot.plot(  
                inputUnit,
                approaches[i],
                marker=markers[i],
                markersize=18,
                linewidth=7,
                color=colors[i],
                )
                handles.append(h)
                legends.append(approachLabels[i])

    mplplot.legend(handles, legends, loc='best', fontsize=fontsize)
   
    sns.despine() 
    ax = fig.gca()
    # Make thicker the spines
    mplplot.setp(ax.spines.values(), linewidth=3)
    # Make thicker the ticks
    ax.xaxis.set_tick_params(width=3)
    ax.yaxis.set_tick_params(width=3)

    xmin, xmax = ax.get_xlim()
    ymin, ymax = ax.get_ylim()

    if specificSettings == "figure_8_1":
        #customize axes ranges
        ax.set(xlim=(100, xmax), ylim=(-10, ymax)) #0.8

    if specificSettings == "figure_8_2":
        mplplot.yticks([0,0.25,0.50,0.75,1,1.25,1.5,1.75,2])

    if specificSettings == "figure_9_1":
        ax.set(xlim=(400, 900), ylim=(-10, ymax))
        mplplot.yticks([0,50,100,150,200,250,300])
        mplplot.xticks([400,500,600,700,800])


    if specificSettings == "figure_10_1":
        ax.set(xlim=(xmin, xmax), ylim=(0.005, ymax))

    if specificSettings == "figure_12_1": 
        ax.set(xlim=(0, 21), ylim=(0, 11))  
        #custom x ticks 
        mplplot.xticks([1,2,3,5,10,20])
    

    if specificSettings == "figure_13_1":
        ax.set(xlim=(xmin, 2.3), ylim=(0, 58))


    if specificSettings == "figure_14_1":
        ax.set(xlim=(0, 2050), ylim=(0, 25))
        mplplot.xticks([0,250,500,1000,2000])

    

    plot.set_xlabel(axisLabels[0], fontsize=fontsize, fontweight='heavy')
    mplplot.xticks(fontsize=fontsize)

    plot.set_ylabel(axisLabels[1], fontsize=fontsize, fontweight='heavy')
    mplplot.yticks(fontsize=fontsize)

    mplplot.tight_layout()
    # mplplot.show()
    
    if save:
        fig.savefig(figName, facecolor=fig.get_facecolor(),
                    edgecolor='none', dpi=200) #bbox_inches='tight'

def box_plot(data_to_plot, xlabels, figName, specificSettings):
    # Create a figure instance
    fig = mplplot.figure(figsize=(12, 9), dpi=200)
    fontsize = 35

    with sns.axes_style("white"):
        sns.set_style("ticks")
        sns.set_context("talk") 
        # Create an axes instance
        plot = fig.add_subplot(111)

        # Create the boxplot
        bp = plot.boxplot(data_to_plot,0, '') #whis = 'range'

        ## change color and linewidth of the medians
        for median in bp['medians']:
            median.set(color='#e7298a', linewidth=3)
        for box in bp['boxes']:
            box.set(linewidth=3)
        for whisker in bp['whiskers']:
            whisker.set(linewidth=3)
        for cap in bp['caps']:
            cap.set(linewidth=3)

    sns.despine()
    ax = fig.gca() 
    # The spines
    mplplot.setp(ax.spines.values(), linewidth=3)
    # The ticks
    ax.xaxis.set_tick_params(width=3)
    ax.yaxis.set_tick_params(width=3)

    # custom x-axis labels
    ax.set_xticklabels(xlabels)
    pylab.ylim(ymin=0)

    if specificSettings == "twitter":
        pylab.ylim(ymax=5)

    mplplot.xlabel("Error bound (in meters)", fontsize=fontsize, fontweight ='heavy')
    mplplot.xticks(fontsize=fontsize)

    mplplot.ylabel("Percent Error ($\%$)", fontsize=fontsize, fontweight ='heavy')
    mplplot.yticks(fontsize=fontsize)

    # Save the figure
    fig.savefig(figName, facecolor=fig.get_facecolor(), 
        edgecolor='none', bbox_inches='tight', dpi=200)

    mplplot.close()

def scatter_error_plot(figName, zoom, y_err_low, y_err_high, accurate_div, approximate_div):    
    fontsize = 35
        
    with sns.axes_style("white"):
        sns.set_style("ticks")
        sns.set_context("talk") 
        # Create an axes instance   

        fig, ax = mplplot.subplots(figsize=(12, 9), dpi=200)
        ax.errorbar(accurate_div, approximate_div, yerr =[y_err_low, y_err_high], fmt='o', markersize=8, linewidth = 3, ecolor='r', color = 'b')
        
        #xmin, xmax = ax.get_xlim()
        ymin, ymax = ax.get_ylim()
        if zoom == 0:
            ax.set(xlim=(0, ymax), ylim=(0, ymax))
        if zoom == 1:
            ax.set(xlim=(0.07, 0.28), ylim=(0.07, 0.28))
        

        diag_line, = ax.plot(ax.get_xlim(), ax.get_ylim(), ls="--", c=".3", linewidth = 3)
        sns.despine() 
        # The spines
        mplplot.setp(ax.spines.values(), linewidth=3)
        # The ticks
        ax.xaxis.set_tick_params(width=3)
        ax.yaxis.set_tick_params(width=3)
    
    if zoom == 0:
        mplplot.xlabel("Accurate Aggregates ($x10^8$)", fontsize=fontsize, fontweight ='bold')
        mplplot.ylabel("Approximate Aggregates ($x10^8$)", fontsize=fontsize, fontweight ='bold')

    if zoom == 1:
        mplplot.xticks(np.arange(0.07, 0.3, 0.05))
        mplplot.yticks(np.arange(0.07, 0.3, 0.05))

    mplplot.xticks(fontsize=fontsize)
    mplplot.yticks(fontsize=fontsize)

    fig.savefig(figName, facecolor='white', 
        edgecolor='none', bbox_inches='tight', dpi=200)

    mplplot.close()
