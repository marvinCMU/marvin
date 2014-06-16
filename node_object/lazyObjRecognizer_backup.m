addpath(genpath(pwd));

speech_folder = '~/data/speech/';
vision_folder = '~/data/vision2/';
detection_images = '~/data/vision_processed/';
detected_models = '~/data/model_processed/'; 
log = '~/data/logging/log.txt';
history = '~/data/logging/history.txt';

%topK = 4;
WINDOW_SIZE = 1;
cmds = {'information' 'option' 'review'};

% initialization
istcLoad;
cls = evalin('caller', 'cls');
models = evalin('caller', 'models');
load confusion_matrix.mat;
[O C CMsc CMso CMvo] = normalizeBNmat();



    % wait for speech
    speech = [];
    while isempty(speech)
       pause(0.01);
       speech = dir([speech_folder 's*']);
    end
    
    speech_file = speech(1).name;
    sc = '';
    while strcmpi(sc, '')
        fid = fopen([speech_folder speech_file]);
        % scenario = fscanf(fid, '%s', 1);
        sc = fscanf(fid, '%s', 1);
        so = fscanf(fid, '%s', 1);
        fclose(fid);
    end

    % corresponding frame (i.e. same timestamp)
    timestamp = regexp(speech_file, '\d+_\d+_\d+', 'match');
    curr_ts = timestamp{1};
    hms = regexp(curr_ts, '_', 'split');
    h = hms(1);
    h = h{1};
    m = hms(2);
    m = m{1};
    s = hms(3);
    s = s{1};
    secs = str2double(h)*3600 + str2double(m)*60 + str2double(s);
    
    sc_ind = find(strcmpi(cmds(:), sc));
    so_ind = find(strcmpi(cls(:), so));
    
    bnp = -1;
    for i=0:WINDOW_SIZE-1 
        h = floor(secs/3600);
        m = floor(mod(secs, 3600)/60); 
        s = mod(secs, 60);
        
        ts = sprintf('%02i_%02i_%02i', h, m, s);
        frame = [vision_folder 'v_' ts '.jpg'];
        while ~exist(frame, 'file') 
        end
        
        [vo_tmp, bno_tmp, bnc_tmp, bnp_tmp, I_tmp, topboxes_tmp, ind4plot_tmp] = ...
            evalFrame(...
            frame, sc, so, sc_ind, so_ind, O, C, ...
            CMsc, CMso, CMvo, cls, models);
        if bnp < bnp_tmp 
            vo = vo_tmp;
            bno = bno_tmp;
            bnc = bnc_tmp;
            bnp = bnp_tmp;
            I = I_tmp;
            topboxes = topboxes_tmp;
            ind4plot = ind4plot_tmp;
            if bnp_tmp == 1
                break;
            end
        end
        secs = secs - 1;
    end
    
    colors = jet(size(topboxes,1));
    colors = colors(end:-1:1,:);
    set(gcf, 'visible', 'off');
    clf;
    imshow(I, 'Border', 'tight');
    if ind4plot ~= -1
        myPlotBox(topboxes(ind4plot,:),colors(ind4plot,:),3);
    end
    saveas(gcf, [detection_images 't_' curr_ts '.jpg']);
    pause(0.01);
    clf;
    if ind4plot ~= -1
        padd = 20;
        indx = topboxes(ind4plot, 6);
        imshow(pad_image(models{indx}.exI, padd, colors(ind4plot,:)));
	else
		imshow(ones(15,20), 'Border', 'tight');
    end
    saveas(gcf, [detected_models 'm_' curr_ts '.jpg']);
    pause(0.01);
    
    % logging 
    fid = fopen(log, 'w');
    fprintf(fid, '%s\n%s %s\n%s\n%s %s\n', curr_ts, sc, so, vo, bnc, bno);
    fclose(fid);
%     fid = fopen(history, 'a');
%     fprintf(fid, '%s\nS: %s %s\nV: %s\nBN: %s %s\n\n', timestamp{1}, sc, so, vo, bnc, bno);
%     fclose(fid);
    
    % delete all speech files
    %arrayfun(@(x) delete([speech_folder x.name]), dir([speech_folder 's*']), ...
    %    'UniformOutput', false);


