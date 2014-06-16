% Do obj recognition on given frame and select obj in the frame based on
% Bayes net
%
% Inputs:
% - sc = speech-recognized command
% - so = speech-recognized obj
% - sc_ind = index to cmds 
% - so_ind = index to cls
% - O, C, CMsc, CMso, CMvo = matrices associated with nodes in Bayes net
%
% Outputs:
% - vo = vision-recognized obj
% - bno = guessed obj from Bayes net
% - bnc = guessed cmd from Bayes net
% - bnp = probability associated with selected obj (bno) and cmd (bnc)
% - I = displayable frame
% - ind4plot = index to a box to be plot in topboxes
function [vo, bno, bnc, bnp, I, topboxes, ind4plot] = evalFrame(frame, sc, ... 
    so, sc_ind, so_ind, O, C, CMsc, CMso, CMvo, cls, models)

nmsthre = 0.5;

cmds = {'information' 'option' 'review'};

num_obj = size(cls,2);

I = convert_to_I(imread(frame));
%find best detections
boxes = my_find_exemp(I, models);
% non-max supression
%TODO: Denver noticed that this is already being done in my_find_exemp?
topboxes = nms(boxes, nmsthre);
%topboxes = boxes;
fprintf('Done running exemplar code. Found %i boxes.\n',size(topboxes,1))

% if at least 1 obj is recognized within the frame
if ~isempty(topboxes)
    num_boxes = size(topboxes, 1);
    % get indices to cls of obj in topboxes
    vos = arrayfun(@(x) models{topboxes(x, 6)}.cls, ... 
        (1:num_boxes)', 'UniformOutput', false);
    
    ind2vos = find(strcmpi(vos(:), so));
    % if one of the objs detected by vision is the same as speech-detected
    % obj
    if ~isempty(ind2vos)
        ind4plot = ind2vos(1);
        vo = so;
        bnc = sc;
        bno = so;
        bnp = 1;
        fprintf('Found %s object in both speech and vision. Using it.',so)
    % if not then 
    % (1) (each obj detected in the frame, speech-recognized obj 
    % and cmd) -> Bayes net 
    % (2) pick the result from BN with highest probability
    else
        fprintf('Did not find speech object %s in vision, diverting to BN.',so)
        vo = vos{1};
        vo_ind = find(strcmpi(cls(:), vo));
        
        [bnc_ind bno_ind bnp] = bayesNet(O, C, CMsc, CMso, CMvo, ...
                sc_ind, so_ind, vo_ind);
        ind4plot = 1;
        for i=2:num_boxes
            vo_tmp= vos{i};
            vo_ind = find(strcmpi(cls(:), vo_tmp));
            [bnc_ind_tmp bno_ind_tmp bnp_tmp] = bayesNet(O, C, CMsc, CMso, CMvo, ...
                sc_ind, so_ind, vo_ind);
            if bnp_tmp > bnp
                vo = vo_tmp;
                bnc_ind = bnc_ind_tmp;
                bno_ind = bno_ind_tmp;
                bnp = bnp_tmp;
                ind4plot = i;
            end
        end
          
        if bno_ind > num_obj
            bnc = 'none';
            bno = 'none';
        else
            bnc = cmds(bnc_ind);
            bnc = bnc{1};
            bno = cls(bno_ind);
            bno = bno{1};
        end
    end
% when no obj is recognized by vision  
else
    fprintf('No object found by vision. Going to BN.')
    
    vo = 'none';
    ind4plot = -1;

    [bnc_ind bno_ind bnp] = bayesNet(O, C, CMsc, CMso, CMvo, ...
        sc_ind, so_ind, num_obj+1);

    if bno_ind > num_obj
            bnc = 'none';
            bno = 'none';
    else
        bnc = cmds(bnc_ind);
        bnc = bnc{1};
        bno = cls(bno_ind);
        bno = bno{1};
    end
end
