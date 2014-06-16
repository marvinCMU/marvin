vidExPipe;
%%
% 1    'apple_adapter'
% 2    'boot1'
% 3    'bottle_dad'
% 4    'bottle_green'
% 5   'bottle_mom'
% 6    'bottle_powder'
% 7    'bottle_red'
% 8    'box_fuel'
% 9    'box_tube'
% 10    'cap1'
% 11    'halls_red'
% 12    'movie_dvd1'
% 13    'toy_dog'
% 14   'umbrella_black'
% 15    'umbrella_green'

models = {};
% for i = [1 2 3 4 5 8 10 12 13 15] %for slow...
for i = [1 4 8 10 12] %for fast...
    models = cat(2, models, load_all_models(dataset_params, cls{i}, ...
        [models_name '-svm'], [], 1, 1));
end
%% sub-Script to read files written by OpenCV!

lastIndexFile = '/home/abhinav/research/codes/openCVCodes/lastIndex.txt';
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