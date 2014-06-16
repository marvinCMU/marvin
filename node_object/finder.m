% Given speech-recognized object, find that obj within the corresponding
% frame (load only that obj model)
addpath(genpath(pwd));

speech_folder = '~/data/speech/';
vision_folder = '~/data/vision/';

while 1 
    % wait for speech
    speech = [];
    while isempty(speech)
        speech = dir([speech_folder 's*']);
    end
    
    % what user say
    speech_file = speech(1).name;
    fid = fopen([speech_folder speech_file]);
    cmd = fscanf(fid, '%s', 1);
    so = fscanf(fid, '%s', 1);
    fclose(fid);
    
    % corresponding frame (i.e. same timestamp)
    timestamp = regexp(speech_file, '\d+_\d+_\d+', 'match');
    vision_file = [vision_folder 'v_' timestamp{1} '.jpg'];
    
    load istc_try_demo.mat;
    %load model
    obj_ind = find(strcmpi(cls(:), so));
    obj_ind = [obj_ind];
    istcInit;
    
    vo = findObj(vision_file, timestamp);
    
    % logging 
    fid = fopen('~/data/logging/log.txt', 'w');
    fprintf(fid, '%s\n%s %s\n%s\n%s %s\n', ...
        timestamp{1}, cmd, so, vo, cmd, so);
    fclose(fid);
    
    % delete all speech files
    arrayfun(@(x) delete([speech_folder x.name]), dir([speech_folder 's*']), ...
        'UniformOutput', false);
end