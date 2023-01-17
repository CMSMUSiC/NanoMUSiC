#!/bin/env python

import sys
import ROOT
import numpy as np
from rootpy.plotting import Hist, Hist2D, Hist3D, HistStack, Legend, Canvas, Graph
from rootpy.plotting.style import get_style, set_style
from rootpy.plotting.utils import get_limits
from rootpy.interactive import wait
import random
import matplotlib
from matplotlib import rc
import matplotlib.ticker as mticker
import rootpy.plotting.root2matplotlib as rplt
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, MultipleLocator

matplotlib.rcParams.update({'font.size': 10})
rc('text', usetex=True)

def hist_example():
    # create a simple 1D histogram with 10 constant-width bins between 0 and 1
    h_simple = Hist(10, 0, 1)
    print h_simple.name
    
    # If the name is not specified, a UUID is used so that ROOT never complains
    # about two histograms having the same name.
    # Alternatively you can specify the name (and the title or any other style
    # attributes) in the constructor:
    h_simple = Hist(10, -4, 12, name='my hist', title='Some Data',
                    drawstyle='hist',
                    legendstyle='F',
                    fillstyle='/')
    
    # fill the histogram
    for i in xrange(1000):
        # all ROOT CamelCase methods are aliased by equivalent snake_case methods
        # so you can call fill() instead of Fill()
        h_simple.Fill(random.gauss(4, 3))
    
    
    # easily set visual attributes
    h_simple.linecolor = 'blue'
    h_simple.fillcolor = 'green'
    h_simple.fillstyle = '/'
    
    # attributes may be accessed in the same way
    print h_simple.name
    print h_simple.title
    print h_simple.markersize
    
    # plot
    canvas = Canvas(width=700, height=500)
    canvas.SetLeftMargin(0.15)
    canvas.SetBottomMargin(0.15)
    canvas.SetTopMargin(0.10)
    canvas.SetRightMargin(0.05)
    h_simple.Draw()
    
    # create the legend
    legend = Legend([h_simple], pad=canvas,
                    header='Header',
                    leftmargin=0.05,
                    rightmargin=0.5)
    legend.Draw()
    
    # 2D and 3D histograms are handled in the same way
    # the constructor arguments are repetitions of #bins, left bound, right bound.
    h2d = Hist2D(10, 0, 1, 50, -40, 10, name='2d hist')
    h3d = Hist3D(3, -1, 4, 10, -1000, -200, 2, 0, 1, name='3d hist')
    
    # variable-width bins may be created by passing the bin edges directly:
    h1d_variable = Hist([1, 4, 10, 100])
    h2d_variable = Hist2D([2, 4, 7, 100, 200], [-100, -50, 0, 10, 20])
    h3d_variable = Hist3D([1, 3, 10], [20, 50, 100], [-10, -5, 10, 20])
    
    # variable-width and constant-width bins can be mixed:
    h2d_mixed = Hist2D([2, 10, 30], 10, 1, 5)
    
    # wait for you to close all open canvases before exiting
    # wait() will have no effect if ROOT is in batch mode:
    # ROOT.gROOT.SetBatch(True)
    wait()

def mpl_example():
    # set the random seed
    ROOT.gRandom.SetSeed(42)
    np.random.seed(42)
    
    # points
    x = np.sort(np.random.random(10)) * 3500
    y = np.random.random(10)
    
    # set style for ROOT
    set_style('CMSTDR')
    
    # create graph
    graph = Graph(x.shape[0])
    for i, (xx, yy) in enumerate(zip(x, y)):
        graph.SetPoint(i, xx, yy)
    
    # set visual attributes
    graph.linecolor = 'blue'
    graph.markercolor = 'blue'
    graph.xaxis.SetTitle("E_{T} [GeV]")
    graph.yaxis.SetTitle("d#sigma_{jet}/dE_{T,jet} [fb/GeV]")
    graph.xaxis.SetRangeUser(0, 3500)
    graph.yaxis.SetRangeUser(0, 1)
    
    # plot with ROOT
    canvas = Canvas()
    graph.Draw("APL")
    
    label = ROOT.TText(0.4, 0.8, "ROOT")
    label.SetTextFont(43)
    label.SetTextSize(25)
    label.SetNDC()
    label.Draw()
    canvas.Modified()
    canvas.Update()
    
    # plot with matplotlib
    
    def plot_with_matplotlib():
        fig, axes = plt.subplots()
    
        axes.plot(x, y, 'o-', markeredgewidth=0)
        axes.set_xlabel(r"$E_T$ [GeV]",
                        horizontalalignment="right", x=1, labelpad=20)
        axes.set_ylabel(r"$d\sigma_{jet}/dE_{T,jet}$ [fb/GeV]",
                        horizontalalignment="right", y=1, labelpad=32)
        axes.set_xlim(0, 3500)
        axes.set_ylim(0, 1)
    
        return fig, axes
    
    # plot without style
    fig1, axes1 = plot_with_matplotlib()
    axes1.text(0.4, 0.8, 'matplotlib (no style)',
               verticalalignment='center', horizontalalignment='center',
               transform=axes1.transAxes, fontsize=20)
    
    # plot with ATLAS style
    set_style('ATLAS', mpl=True)
    fig2, axes2 = plot_with_matplotlib()
    axes2.text(0.4, 0.8, 'matplotlib',
               verticalalignment='center', horizontalalignment='center',
               transform=axes2.transAxes, fontsize=20)
    axes2.xaxis.set_minor_locator(AutoMinorLocator())
    axes2.yaxis.set_minor_locator(AutoMinorLocator())
    
    if not ROOT.gROOT.IsBatch():
        plt.show()
    
    # wait for you to close the canvas before exiting
    wait(True)

def r2m_example():
    # set the style
    style = get_style('ATLAS')
    style.SetEndErrorSize(3)
    set_style(style)
    
    # set the random seed
    ROOT.gRandom.SetSeed(42)
    np.random.seed(42)
    
    # signal distribution
    signal = 126 + 10 * np.random.randn(100)
    signal_obs = 126 + 10 * np.random.randn(100)
    
    # create histograms
    h1 = Hist(30, 40, 200, title='Background', markersize=0)
    h2 = h1.Clone(title='Signal')
    h3 = h1.Clone(title='Data')
    h3.markersize = 1.2
    
    # fill the histograms with our distributions
    h1.FillRandom('landau', 1000)
    map(h2.Fill, signal)
    h3.FillRandom('landau', 1000)
    map(h3.Fill, signal_obs)
    
    # set visual attributes
    h1.fillstyle = 'solid'
    h1.fillcolor = 'green'
    h1.linecolor = 'green'
    h1.linewidth = 0
    
    h2.fillstyle = 'solid'
    h2.fillcolor = 'red'
    h2.linecolor = 'red'
    h2.linewidth = 0
    
    stack = HistStack()
    stack.Add(h1)
    stack.Add(h2)

    # plot with ROOT
    canvas = Canvas(width=700, height=500)
    
    # try setting logy=True and uncommenting the two lines below
    xmin, xmax, ymin, ymax = get_limits([stack, h3], logy=False)
    stack.SetMaximum(ymax)
    #stack.SetMinimum(ymin)
    #canvas.SetLogy()
    
    stack.Draw('HIST E1 X0')
    h3.Draw('SAME E1 X0')
    stack.xaxis.SetTitle('Mass')
    stack.yaxis.SetTitle('Events')
    # set the number of expected legend entries
    legend = Legend(3, leftmargin=0.45, margin=0.3)
    legend.AddEntry(h1, style='F')
    legend.AddEntry(h2, style='F')
    legend.AddEntry(h3, style='LEP')
    legend.Draw()
    label = ROOT.TText(0.3, 0.8, 'ROOT')
    label.SetTextFont(43)
    label.SetTextSize(25)
    label.SetNDC()
    label.Draw()
    canvas.Modified()
    canvas.Update()

    # plot with matplotlib
    set_style('ATLAS', mpl=True)
    fig = plt.figure(figsize=(7, 5), dpi=100)
    axes = plt.axes()
    axes.xaxis.set_minor_locator(AutoMinorLocator())
    axes.yaxis.set_minor_locator(AutoMinorLocator())
    axes.yaxis.set_major_locator(MultipleLocator(20))
    rplt.bar(stack, stacked=True, axes=axes)
    rplt.errorbar(h3, xerr=False, emptybins=False, axes=axes)
    plt.xlabel('Mass', position=(1., 0.), va='bottom', ha='right')
    plt.ylabel('Events', position=(0., 1.), va='top', ha='right')
    axes.xaxis.set_label_coords(1., -0.20)
    axes.yaxis.set_label_coords(-0.18, 1.)
    leg = plt.legend()
    axes.text(0.3, 0.8, 'matplotlib',
              verticalalignment='center', horizontalalignment='center',
              transform=axes.transAxes, fontsize=20)
    
    if not ROOT.gROOT.IsBatch():
        plt.show()
        # wait for you to close the ROOT canvas before exiting
        wait(True)

def test():
    # set the style
    style = get_style('ATLAS')
    style.SetEndErrorSize(3)
    set_style(style)
    
    # set the random seed
    ROOT.gRandom.SetSeed(42)
    np.random.seed(42)
    
    # signal distribution
    signal = 126 + 10 * np.random.randn(1000)
    signal_obs = 126 + 10 * np.random.randn(1000)
    
    # create histograms
    h1 = Hist(30, 40, 200, title='Background', markersize=0)
    h2 = h1.Clone(title='Signal')
    h3 = h1.Clone(title='Data')
    h3.markersize = 1.2
    
    # fill the histograms with our distributions
    h1.FillRandom('landau', 10000)
    map(h2.Fill, signal)
    h3.FillRandom('landau', 10000)
    map(h3.Fill, signal_obs)
    
    # set visual attributes
    h1.fillstyle = 'solid'
    h1.fillcolor = 'green'
    h1.linecolor = 'green'
    h1.linewidth = 0
    
    h2.fillstyle = 'solid'
    h2.fillcolor = 'red'
    h2.linecolor = 'red'
    h2.linewidth = 0
    
    stack = HistStack()
    stack.Add(h1)
    stack.Add(h2)

    # plot with matplotlib
    fig = plt.figure(figsize=(6, 6), dpi=100, facecolor='w')

    ax1 = plt.subplot2grid((6,4), (1,0), rowspan=4, colspan=4)
    ax1.xaxis.set_minor_locator(AutoMinorLocator())
    ax1.yaxis.set_minor_locator(AutoMinorLocator())
    ax1.yaxis.set_major_locator(MultipleLocator(20))
    rplt.bar(stack, stacked=True, axes=ax1)
    rplt.errorbar(h3, xerr=False, emptybins=False, axes=ax1, markersize=4)
    plt.xlabel('Mass (GeV)', position=(1., -0.1), va='top', ha='right')
    plt.ylabel('Events', position=(0.1, 1.), va='top', ha='right')
    leg = plt.legend(numpoints=1)
    leg.get_frame().set_alpha(0.0)
    #ax1.set_yscale('symlog')
    ax1.yaxis.set_major_locator(mticker.MaxNLocator(prune='lower'))
    
    ax0 = plt.subplot2grid((6,4), (0,0), rowspan=1, colspan=4, sharex=ax1)
    sig_hist = h1.Clone(title='signi')
    for i in range(sig_hist.GetNbinsX()+1):
        value = float(h3.GetBinContent(i) - h1.GetBinContent(i))/np.sqrt(float(pow(h3.GetBinError(i),2) + pow(h1.GetBinError(i),2)))
        sig_hist.SetBinContent(i,value)
        sig_hist.SetBinError(i,1)
    rplt.errorbar(sig_hist, xerr=False, emptybins=False, axes=ax0, markersize=4, ecolor='black')
    plt.ylabel('Significance', position=(0., 1.), va='top', ha='right')
    ax0.axhline(0, color='blue')
    ax0.yaxis.set_major_locator(mticker.MaxNLocator(nbins=5, prune='lower'))

    ax2 = plt.subplot2grid((6,4), (5,0), rowspan=1, colspan=4, sharex=ax1)
    sum_hist = h1.Clone(title='sum')
    sum_hist.Add(h2)
    ratio_sig = h3.Clone(title='ratio_sig')
    ratio_sig.Divide(sum_hist)
    ratio_1 = rplt.errorbar(ratio_sig, xerr=False, emptybins=False, axes=ax2, ecolor='red', markersize=4, label='Data/(Bag+Sig)')
    ratio = h3.Clone(title='ratio')
    ratio.Divide(h1)
    ratio_2 = rplt.errorbar(ratio, xerr=False, emptybins=False, axes=ax2, ecolor='green', markersize=4, label='Data/Bag')
    leg = plt.legend(loc=2, ncol=1, prop={'size':8}, borderaxespad=0., numpoints=1)
    leg.get_frame().set_alpha(0.0)
    ax2.yaxis.set_major_locator(mticker.MaxNLocator(nbins=5, prune='upper'))
    ax2.axhline(1, color='blue')
    plt.ylabel('Ratio Data/MC', position=(0., 1.), va='top', ha='right')
    plt.xlabel('Mass (GeV)', position=(1., -0.1), va='top', ha='right')

    plt.subplots_adjust(left=.08, bottom=.10, right= .92, top=.95, wspace =.2, hspace=.0)
    plt.setp(ax0.get_xticklabels(), visible=False)
    plt.setp(ax1.get_xticklabels(), visible=False)

    fig.text(0.92, 0.955, '$42\,\mathrm{fb^{-1}} (13\,\mathrm{TeV})$', va='bottom', ha='right', color='black', size=12)
    fig.text(0.10, 0.915, 'CMS', va='bottom', ha='left', color='black', size=14, weight='bold')
    fig.text(0.10, 0.885, '$Preliminary$', va='bottom', ha='left', color='black', size=10)
    
    if not ROOT.gROOT.IsBatch():
        plt.savefig('test_plt.pdf',facecolor=fig.get_facecolor())
        # wait for you to close the ROOT canvas before exiting
        wait(True)

def main():
    if len(sys.argv) > 1:
        if 'hist' in sys.argv:
            hist_example()
        if 'mpl' in sys.argv:
            mpl_example()
        if 'r2m' in sys.argv:
            r2m_example()
        if 'test' in sys.argv:
            test()

main()
