addpath(genpath(pwd));
vidExPipe;
%%
% 1    'appliances'
% 2    'boxes_translucent'
% 3    'can_green'
% 4    'can_white'
% 5   'coffee_maker'
% 6    'cutlery_box'
% 7    'oil'
% 8    'salt_box'
% 9    'salt_can'
% 10    'salt_whitebox'
% 11    'spray_can'
% 12    'spray_cleaner'
% 13    'wipes'

models = {};
for i = [3]
% for i = [7:10] %
    models = cat(2, models, load_all_models(dataset_params, cls{i}, ...
        [models_name '-svm'], [], 1, 1));
end
%% sub-Script to read files written by OpenCV!

lastIndexFile = '/home/abhinav/research/codes/opencvFPV/lastIndex.txt';
oldFname{1} = '';

fName = textread(lastIndexFile,'%s');
while 1
    while strcmp(oldFname{1}, fName{1})
        while 1
            try
                fName = textread(lastIndexFile,'%s');
                break
            catch e
                continue;
            end
        end
    end
    I = convert_to_I(imread(fName{1}));
    my_find_exemp(I, models);
    pause(0.01);
    oldFname = fName;
end


%%
sz = '1280x960';
device = '/dev/video3';

while 1
    tic
    unix(sprintf('streamer -q -s %s -c %s -f jpeg -o %simg.jpeg',sz,device,VOCopts.dumpdir));
    toc
    
    I = convert_to_I(imread(fullfile(VOCopts.dumpdir,'img.jpeg')));
    my_find_exemp(I, models);
    drawnow
    pause(0.001);
end