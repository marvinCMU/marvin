function myPlotBox(bbox, c, cwidth)
x1 = bbox(1);
x2 = bbox(3);
y1 = bbox(2);
y2 = bbox(4);
line([x1 x1 x2 x2 x1]', [y1 y2 y2 y1 y1]', 'color', c, 'linewidth', cwidth);