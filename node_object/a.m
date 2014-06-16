close all;

%% sub-Script to read files written by OpenCV!
figure(1)
clf
set(gcf,'color','k');

lastIndexFile = '/home/abhinav/research/codes/opencvFPV/lastIndex.txt';
oldFname = '';

out_path = '/home/abhinav/research/vision_results';
if ~exist(out_path, 'dir')
    mkdir(out_path);
end

[fName, timestamp] = textread(lastIndexFile,'%s %s', 1);
topK = 4;
while 1
    while strcmp(oldFname, fName{1})
        while 1
            try
                [fName, timestamp] = textread(lastIndexFile,'%s %s', 1);
                break
            catch e
                continue;
            end
        end
    end
    I = convert_to_I(imread(fName{1}));
    %find best detections
    boxes = my_find_exemp(I, models);
    topboxes = nms(boxes, 0.5);
    topboxes = topboxes(1:min(topK,size(topboxes,1)),:);
    %display detections
    show_res_video(boxes, I, models);
    outfname = [out_path '/v_' timestamp{1}];
    saveas(gcf, [outfname '_pic.jpg']);
    fid = fopen([outfname '.txt'], 'w');
    arrayfun(@(x) fprintf(fid, '%s\n', models{topboxes(x, 6)}.cls), ...
        1:size(topboxes, 1), ...
        'UniformOutput', false);
    
    fclose(fid);
    pause(0.01);
    oldFname = fName{1};
end