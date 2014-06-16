% offlineDetector detects object in each frame of the video using given 
% models and build a vector that counts how many times each obj is detected  

% Inputs:
% - vid: file path of video
% - models
% - cls
% - visual: 1 if want detection images 0 o.w.
%
% Outputs:
% - v: Column correspond to those of cls. 
%   The number in each column counts how many times the obj is detected.
%   Last column is number of times no obj is recognized.
% - a directory that contains all frames extracted from video
% - (if visual == 1) a directory that contains all detection results of the 
% frames
function v = offlineDetector(vid, models, cls, visual)
v = zeros(1, size(cls, 2)+1);
if strcmp(vid, '')
    return
end

%% extract frames from video and store all the frames in directory with
% the same name as video
[path, name, ext] = fileparts(vid);
output_path = '~/data/CM_videos/frames/';
frame_path = [output_path name '/frame'];
if ~exist([output_path name], 'dir')
    mkdir([output_path name]);
    system(['ffmpeg -i ' vid ' ' frame_path '%d.jpg']);
end

%% do detection on each frame and save the detection results into a folder
if visual
    results_dir = [output_path name '_results'];
    if exist(results_dir, 'dir')
        rmdir(results_dir, 's');
    end
    mkdir(results_dir);
    
    figure(1)
    clf
    set(gcf,'color','k');
end 

num_frames = numel(dir([frame_path '*.jpg']));
topK = 4;
for i = 1:num_frames
    I = convert_to_I(imread([frame_path int2str(i) '.jpg']));
    % find best detections
    boxes = my_find_exemp(I, models);
    % TODO: The next 2 lines are the same as those in show_res_video.
    topboxes = nms(boxes, 0.5);
    %topboxes = topboxes(1:min(topK,size(topboxes,1)),:);
  
    if ~isempty(topboxes)
        guess_inds = cell2mat(arrayfun(@(x) find(strcmpi(cls(:), ...
            models{topboxes(x, 6)}.cls)), 1:size(topboxes, 1), ...
            'UniformOutput', false));
        v(guess_inds) = v(guess_inds) + 1;
    else
        v(end) = v(end) + 1;
    end
    
    if visual 
        % display detections
        show_res_video(boxes, I, models);
        saveas(gcf, [results_dir '/frame' int2str(i) 'd.jpg']);
        pause(0.01);
    end 
end