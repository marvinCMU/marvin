% Build confusion matrix for given obj(s) 
%
% Inputs:
% - vids: column cell of path to videos. The video in each column  
%   corresponds to obj in the same column in cls. If don't want to 
%   evaluate a certain obj use ''
% - visual: boolean indicated if you want to save images of detection 
%   results
%
% Outputs: 
% - confusion_mat: a confusion matrix 
function confusion_mat = confusion(vids, models, cls, visual)
close all;

confusion_mat = cell2mat(cellfun(@(x) ...
    offlineDetector(x, models, cls, visual), vids, 'UniformOutput', false));



