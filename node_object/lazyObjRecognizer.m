addpath(genpath(pwd));
display('Lazy Object Recognizer starting!')
data_path = '/home/mmfps/mmfps/mmfpspkg/data_tmp/'
speech_folder = [data_path,'speech/'];
vision_folder = [data_path,'vision/'];
detection_images = [data_path,'vision_processed/'];
detected_models = [data_path,'model_processed/'];
log = [data_path,'gui/log.txt'];
indicator_file = [data_path,'logging/done']
history = '/home/mmfps/mmfps/mmfpspkg/data_tmp/logging/history.txt';

%topK = 4;
WINDOW_SIZE = 3;
cmds = {'information' 'option' 'review'};

% initialization
istcLoad;
models = models(34:249);


% cls = evalin('caller', 'cls');
% models = evalin('caller', 'models');
load confusion_matrix.mat;
[O C CMsc CMso CMvo] = normalizeBNmat();
fprintf('Waiting for speech file.\n')
while 1 
    no_file = 0;
    % wait for speech
    speech = [];
    while isempty(speech)
       pause(0.01);
       speech = dir([speech_folder 's*']);
    end
    
    speech_file = speech(1).name;
    fprintf('Found speech file %s.\n', speech_file)
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
    i= 0;
    while i <= WINDOW_SIZE-1 %for i=0:WINDOW_SIZE-1 
        h = floor(secs/3600);
        m = floor(mod(secs, 3600)/60); 
        s = mod(secs, 60);
        
        ts = sprintf('%02i_%02i_%02i', h, m, s);
        frames_looked = 0;
        frame = [vision_folder 'v_' ts '.jpg'];
        fprintf('Looking for frame %s\n',frame)
		cur_time = now;
		max_time = 2e-5;
                
        if~exist(frame, 'file')           
            display('using unix command')
             [status, frame] = unix(['find /home/mmfps/mmfps/', ...
                                 'mmfpspkg/data_tmp/vision -type f ',...
                                 '-printf "T@ %p\n" | sort -n | tail -1 | cut -f2- -d" "'])
             frame = frame(1:end-1);
        end

% $$$          while ~exist(frame, 'file') && frames_looked<3
% $$$              now-cur_time;
% $$$ 		if abs(now-cur_time) >= max_time
% $$$ 		     secs = s +1;
% $$$ 		     s = mod(secs, 60);
% $$$ 		     m = mod(m + floor(secs/60),60); 
% $$$ 		     h = h+floor((m+floor(secs/60))/60);
% $$$ 		     ts = sprintf('%02i_%02i_%02i', h, m, s);
% $$$ 		     frame = [vision_folder 'v_' ts '.jpg'];
% $$$ 		     fprintf('time elapsed, looking for frame %s\n', frame)
% $$$ 		     frames_looked = frames_looked +1;
% $$$                      cur_time = now;
% $$$ 		end
% $$$ 		
% $$$         end
% $$$         if ~exist(frame, 'file')
% $$$             display('not there')
% $$$             [status, frame] = unix(['find /home/mmfps/mmfps/' ...
% $$$                                 'mmfpspkg/data_tmp/vision | sort -n | tail -1'])
% $$$     
% $$$         end

        
         if isempty(strfind(frame, '.jpg'))
             no_file = 1;
         end
        
         if no_file == 0
             fprintf('Found frame %s.\n',frame)
             
             %evalFrame does vision processing And the BN processing. For
             %now as a hack I'll just take the vo_tmp output and save it to
             %disk somewhere
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
       
         i = i + 1;
    end
    
    if no_file == 0
        colors = jet(size(topboxes,1));
        colors = colors(end:-1:1,:);
        set(gcf, 'visible', 'off');
        clf;
        imshow(I, 'Border', 'tight');
        org_img = imread(frame);
        x_ratio = size(org_img,1) / size(I,1);
        y_ratio = size(org_img,2) / size(I,2);
        ratio_vec = [x_ratio, y_ratio, x_ratio, y_ratio];
        if ind4plot ~= -1
            myPlotBox(topboxes(ind4plot,:),colors(ind4plot,:),3);
        end
        bbox = topboxes(ind4plot,:);
        bbox = bbox(1:4) .* ratio_vec;
        saveas(gcf, [detection_images 't_' curr_ts '.jpg']);
        %   [img, map] = getframe(gcf);
        %    imwrite(img, [detection_images 't_' curr_ts '.jpg'], 'Quality', 0);
        %		close;


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
        %   [img, map] = getframe(gcf);
        %   imwrite(img, [detected_models 'm_' curr_ts '.jpg'], 'Quality', 0);
        %  pause(0.01);
        close;
        
        % logging 
        fid = fopen(log, 'w');
        filename = sprintf('%s\n%s %s\n%s\n%s %s\n', curr_ts, ...
                           sc, so, vo, bnc, bno);

        fprintf(fid, '%s\n%s %s\n%s\n%s %s\n%s\n%s\n%s\n%s\n', curr_ts, sc, ...
                so, vo, bnc, bno, num2str(bbox(1),4), num2str(bbox(2),4), num2str(bbox(3),4), num2str(bbox(4),4));
        display(['log file', log, 'is being updated to', fid])
        fclose(fid);
        fid = fopen(indicator_file, 'w')
        fprintf(fid, ' ');
        fclose(fid);
        %     fid = fopen(history, 'a');
        %     fprintf(fid, '%s\nS: %s %s\nV: %s\nBN: %s %s\n\n', timestamp{1}, sc, so, vo, bnc, bno);
        %     fclose(fid);
        
        % delete all speech files
        arrayfun(@(x) delete([speech_folder x.name]), dir([speech_folder 's*']), ...
                 'UniformOutput', false);
    end

end

