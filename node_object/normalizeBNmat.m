% Normalize and smooth out each matrix in Bayes net
function [O C CMsc CMso CMvo] = normalizeBNmat()

O = evalin('caller', 'O');
C = evalin('caller', 'C');
CMsc = evalin('caller', 'CMsc');
CMso = evalin('caller', 'CMso');
CMvo = evalin('caller', 'CMvo');

O = normalize(O+ones(size(O)), 1);
C = normalize(C+ones(size(C)), 2);
CMsc = normalize(CMsc+ones(size(CMsc)), 2);
CMso = normalize(CMso+ones(size(CMso)), 2);
CMvo = normalize(CMvo+ones(size(CMvo)), 2);
