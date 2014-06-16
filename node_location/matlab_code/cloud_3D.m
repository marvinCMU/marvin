close all;
clear all;
clc;

%file = fopen ('/home/qifan/CMUstore/reconstruction/structure.txt');
%file = fopen ('/home/qifan/Desktop/structure.txt');
file = fopen ('structure.txt');
data = textscan(file, '%d %d %d %d %f %f %f');

r = data{2};
g = data{3};
b = data{4};
x = data{5};
y = data{6};
z = data{7};

%r = r(1:length(r)/2);
%g = g(1:length(g)/2);
%b = b(1:length(b)/2);
%x = x(1:length(x)/2);
%y = y(1:length(y)/2);
%z = z(1:length(z)/2);

points = [x, y, z];
colors = [r, g, b];

pointSize = 1;
%[colors, map] = rgb2ind(colors, 32);

figure;
scatter3(x, y, z, pointSize, 'filled');
%scatter3(x, y, z, pointSize, colors, 'filled');

%view(-10, -10);