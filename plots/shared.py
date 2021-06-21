import os
import os.path
import numpy
import pandas
import toyplot
import toyplot.pdf

colormap = toyplot.color.CategoricalMap()

def GetGlyphMarkers() :
  toReturn = []
  mshape = 'o'
  marker = toyplot.marker.create(shape=mshape, mstyle={"stroke":"black", "opacity":0.5}, size=8)
  toReturn.append(("Euler", marker))
  mshape = '^'
  marker = toyplot.marker.create(shape=mshape, mstyle={"stroke":"black", "opacity":0.5}, size=8)
  toReturn.append(("RK4", marker))
  return toReturn

def GetColorMarkers():
  toReturn = []
  mshape = 'o'
  marker = toyplot.marker.create(shape=mshape, mstyle={"stroke":colormap.css(0),"fill":colormap.css(0),"opacity":0.5}, size=8)
  toReturn.append(("CPU", marker))
  marker = toyplot.marker.create(shape=mshape, mstyle={"stroke":colormap.css(1),"fill":colormap.css(1),"opacity":0.5}, size=8)
  toReturn.append(("GPU", marker))
  return toReturn

#experiments
exp = [
[100, 100, 100, 1000, 1000, 1000, 10000, 10000, 10000,
 100000,100000,100000, 1000000,1000000,1000000],
[100, 1000, 10000, 100, 1000, 10000, 100, 1000, 10000,
 100, 1000, 10000, 100, 1000, 10000] ]

# 0 : GPU
# 1 : CPU
alaska = [
[5.465116279, 6.162897858, 6.097386594, 5.811249706, 6.55873845, 6.862323318, 6.582276901,
 6.737583123, 6.792464818, 7.11206109,  6.797258419, 6.88955085, 6.931900201, 6.938837877, 6.909411323],
[0.475277361, 0.4854443325, 0.4929625834, 4.117932206, 4.276391096, 4.346728422, 10.44896053, 10.56794146,
10.59531496, 11.79884975, 11.29352993, 11.39020391, 11.48397895, 11.47508256,11.42214809] ]

summit = [
[ 5.836779108,10.34213283,11.25402077,12.52818776,13.6541289,14.68270068, 13.81518973, 14.58104587, 15.45274102,
  15.94051728, 16.13243676, 19.51934096, 16.76677992, 20.18569551, 26.07768749 ],
[ 1.1115859, 1.308779131,1.377458577,7.061864106, 8.303071757, 8.478336891, 30.4738495,
 33.3557671, 33.37001474, 41.40415529, 41.49140872, 41.05081157, 41.86171582, 41.12265551, 40.92636631,] ]

voltar = [
[ 6.186119874, 8.299976117,7.671112221,11.48126597,12.42305051,14.82696737,15.67309224,
 13.49722339,21.27666055,17.65769031,22.41128743,24.76197636,19.02436838,25.57934225,27.25974339 ],
[ 1.651679053,1.26106849,1.192261128,11.14363052,10.12790267,9.786570976, 52.37474969, 35.05924858,
  36.33429423, 55.33468608, 41.03866719, 40.99674167, 42.16343237, 42.44910068, 41.83757709 ] ]

sizes = {100 : 6, 1000 : 7.5, 10000 : 9, 100000 : 10.5, 1000000 : 12}

total = numpy.array(exp[0])*numpy.array(exp[1])

tickstyle = {"font-size":"14px"}

canvas = toyplot.Canvas(width=700, height=600)

axes = canvas.cartesian(grid=(2, 2, 0))
axes.x.scale = "log10"
axes.x.ticks.labels.style = tickstyle
axes.y.ticks.labels.style = tickstyle
#axes.y.spine.show = False
size = list(map(lambda x : sizes[x], exp[0]))
axes.scatterplot(total, numpy.array(alaska[0]), color=([0]*len(total), colormap), size=size, opacity=0.5)
axes.scatterplot(total, numpy.array(alaska[1]), color=([1]*len(total), colormap), size=size, opacity=0.5)
axes.x.label.text = "Alaska (12 threads) / K20C"

axes = canvas.cartesian(grid=(2, 2, 1))
axes.x.scale = "log10"
axes.x.ticks.labels.style = tickstyle
axes.y.ticks.labels.style = tickstyle
#axes.y.spine.show = False
size = list(map(lambda x : sizes[x], exp[0]))
axes.scatterplot(total, numpy.array(voltar[0]), color=([0]*len(total), colormap), size=size, opacity=0.5)
axes.scatterplot(total, numpy.array(voltar[1]), color=([1]*len(total), colormap), size=size, opacity=0.5)
axes.x.label.text = "Voltar (32 threads) / P100"

axes = canvas.cartesian(grid=(2, 2, 2))
axes.x.scale = "log10"
axes.x.ticks.labels.style = tickstyle
axes.y.ticks.labels.style = tickstyle
#axes.y.spine.show = False
size = list(map(lambda x : sizes[x], exp[0]))
axes.scatterplot(total, numpy.array(summit[0]), color=([0]*len(total), colormap), size=size, opacity=0.5)
axes.scatterplot(total, numpy.array(summit[1]), color=([1]*len(total), colormap), size=size, opacity=0.5)
axes.x.label.text = "Summit (32 threads) / V100"

canvas.legend(GetColorMarkers(), label="Device", rect=(600, 450, "5%", "5%"))

toyplot.pdf.render(canvas, "sharedplot.pdf")
