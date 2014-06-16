function [resstruct,feat_pyramid] = esvm_detect_fast(I, models, params)

if isempty(models)
  fprintf(1,'Warning: empty models in esvm_detect\n');
  resstruct.bbs{1} = zeros(0,0);
  resstruct.xs{1} = zeros(0,0);
  feat_pyramid = [];
  return;
end

if ~iscell(models)
  models = {models};
end

if isfield(models{1},'mining_params') && ~exist('params','var')
  params = models{1}.mining_params;
elseif ~exist('params','var')
  params = esvm_get_default_params;
end

if ~isfield(params,'nnmode')
 params.nnmode = '';
end

% get the hog in advance
params.preprocessing_startingtime = tic;
sbin = models{1}.model.init_params.sbin;
params.detect_add_flip = 0;
t(1) = get_pyramid(I, sbin, params);
params.detect_add_flip = 1;
t(2) = get_pyramid(I,sbin,params);
params.precomputation = t;
[rs,feat_pyramid] = esvm_detectdriverBLOCK_fast(models, params);
resstruct = prune_nms(rs, params);
if params.truncatedModels
    delete(params.ranked_list_filename);
end
SVM_time = toc(rs.SVM_startingtime);
fprintf(['SVM running time: ' num2str(SVM_time) '\n']);





function [resstruct,t] = esvm_detectdriverBLOCK_fast(models,params)

%%HERE is the chunk version of exemplar localization

N = length(models);
ws = cellfun2(@(x)x.model.w,models);
bs = cellfun(@(x)x.model.b,models)';
bs = reshape(bs,[],1);
sizes1 = cellfun(@(x)x.model.hg_size(1),models);
sizes2 = cellfun(@(x)x.model.hg_size(2),models);

S = [max(sizes1(:)) max(sizes2(:))];
fsize = params.init_params.features();
exemplar_matrix = params.exemplar_matrix;


t = params.precomputation;
sbin = models{1}.model.init_params.sbin;

resstruct.padder = t(1).padder;
pyr_N = cellfun(@(x)prod([size(x,1) size(x,2)]-S+1),t(1).hog);
sumN = sum(pyr_N);

X_orig = zeros(S(1)*S(2)*fsize,sumN);
X_flip = zeros(S(1)*S(2)*fsize,sumN);

level_orig = cell(length(t(1).hog), 1);
level_flip = cell(length(t(2).hog), 1);

uus_orig = cell(length(t(1).hog),1);
vvs_orig = cell(length(t(1).hog),1);

uus_flip = cell(length(t(2).hog),1);
vvs_flip = cell(length(t(2).hog),1);


counter = 1;
for i = 1:length(t(1).hog)
  s = size(t(1).hog{i});
  NW = s(1)*s(2);
  ppp = reshape(1:NW,s(1),s(2));
  curf_orig = reshape(t(1).hog{i},[],fsize);
  curf_flip = reshape(t(2).hog{i},[],fsize);
  
  b = im2col(ppp,[S(1) S(2)]);

  level_orig{i} = i*ones(1,size(b,2));
  level_flip{i} = i*ones(1,size(b,2));
  
  for j = 1:size(b,2)
   X_orig(:,counter) = reshape(curf_orig(b(:,j),:),[],1);
   X_flip(:,counter) = reshape(curf_flip(b(:,j),:),[],1);
   counter = counter + 1;
  end
  
  [uus_orig{i},vvs_orig{i}] = ind2sub(s,b);
  uus_flip{i} = uus_orig{i};
  vvs_flip{i} = vvs_orig{i};
  
end

uus_orig = cat(2,uus_orig{:});
vvs_orig = cat(2,vvs_orig{:});
uus_flip = cat(2,uus_flip{:});
vvs_flip = cat(2,vvs_flip{:});
uus = [uus_orig uus_flip];
vvs = [vvs_orig vvs_flip];
level = [cat(2,level_orig{:}) cat(2,level_flip{:})];
X = [X_orig X_flip];
clear X_orig X_flip uus_orig uus_flip vvs_orig vvs_flip level_orig level_flip
of_indicator = [zeros(1,sumN) ones(1,sumN)]; % origin/flip indicator




if params.truncatedModels
    sleeping_time = 0.05;
    while ~exist(params.ranked_list_filename,'file')
        pause(sleeping_time);
    end

    rankedList = load(params.ranked_list_filename);
    val_idx = rankedList>=1 & rankedList<=N;
    rankedList = rankedList(val_idx);
    fprintf('found ranked_exemplars.txt\n');
    if isfield(params,'displayRankedList') && params.displayRankedList
        selected_models = params.mapping(rankedList);
        selected_objs = cellfun(@(x)x.obj_name,selected_models,'UniformOutput',false);
        [ignore IA IC] = unique(selected_objs,'first');
        selected_objs = selected_objs(sort(IA,'ascend'));
        fprintf('objs recommended by bnet: ');
        for k = 1:length(selected_objs)
            fprintf([selected_objs{k} '  ']);
        end
        fprintf('\n');
    end
    try
        exemplar_matrix = exemplar_matrix(:,rankedList);
        bs = bs(rankedList);
    catch
        error('ranked list contains invalid model index...');
    end
else
    rankedList = [1:N];
%     exemplar_matrix = exemplar_matrix(:,rankedList);
%     bs = bs(rankedList);
    fprintf('non-truncated mode...\n');
end

preprocessing_time = toc(params.preprocessing_startingtime);
fprintf(['preprocessing finished, took ' num2str(preprocessing_time) 'secs\n']);
fprintf(['number of models: ' num2str(size(exemplar_matrix,2)) '\n']);
resstruct.SVM_startingtime = tic;

if params.GPUacc
    Agpu = gpuArray( [exemplar_matrix' bs] );
    Bgpu = gpuArray( [X;-ones(1,size(X,2))] );
    % Multiply-add on GPU
    r = ( Agpu * Bgpu);
    % this forces the computation, and converts the result to standard Matlab variable
    r = gather( r );
else
    r = exemplar_matrix' * X;
    r = bsxfun(@minus, r, bs);
end


resstruct.bbs = cell(2*N,1);
resstruct.xs = cell(2*N,1);


for i = 1:length(rankedList)
    exid = rankedList(i);

    goods = find(r(i,:) >= params.detect_keep_threshold);
  
    if isempty(goods)
        continue    
    end
    
    goods_orig = goods(goods<=sumN);
    goods_flip = goods(goods>sumN);
    
    bb_orig = [];
    bb_flip = [];
    sorted_scores_orig = [];
    sorted_scores_flip = [];
    
    if ~isempty(goods_orig)
        [sorted_scores_orig,bb_orig] = ...
          psort(-r(i,goods_orig)',...
                min(params.detect_max_windows_per_exemplar, ...
                    length(goods_orig)));
        bb_orig = goods_orig(bb_orig);
    end
    
    if ~isempty(goods_flip)
        [sorted_scores_flip,bb_flip] = ...
          psort(-r(i,goods_flip)',...
                min(params.detect_max_windows_per_exemplar, ...
                    length(goods_flip)));
        bb_flip = goods_flip(bb_flip);
    end
    
    bb = [bb_orig bb_flip];
    
    sorted_scores = -[sorted_scores_orig;sorted_scores_flip]';

    
    levels = level(bb);
    scales = t(1).scales(levels);
    curuus = uus(bb);
    curvvs = vvs(bb);

    o = [curuus' curvvs'] - t(1).padder;

    bbs = ([o(:,2) o(:,1) o(:,2)+size(ws{exid},2) ...
           o(:,1)+size(ws{exid},1)] - 1) .* ...
             repmat(sbin./scales',1,4) + 1 + repmat([0 0 -1 ...
                    -1],length(scales),1);

    bbs(:,5:12) = 0;
    bbs(:,5) = (1:size(bbs,1));
    bbs(:,6) = exid;
    bbs(:,7) = of_indicator(bb);
    bbs(:,8) = scales;
    bbs(:,9) = uus(bb);
    bbs(:,10) = vvs(bb);
    bbs(:,12) = sorted_scores;

    flip_det_idx = bbs(:,7)>0;
    bbs(flip_det_idx,:) = flip_box(bbs(flip_det_idx,:),t(1).size);
    resstruct.bbs{exid} = bbs(~flip_det_idx,:);
    resstruct.bbs{exid+N} = bbs(flip_det_idx,:);
end



function rs = prune_nms(rs, params)
%Prune via nms to eliminate redundant detections

%If the field is missing, or it is set to 1, then we don't need to
%process anything.  If it is zero, we also don't do NMS.
if ~isfield(params,'detect_exemplar_nms_os_threshold') || (params.detect_exemplar_nms_os_threshold >= 1) ...
      || (params.detect_exemplar_nms_os_threshold == 0)
  return;
end

rs.bbs = cellfun2(@(x)esvm_nms(x,params.detect_exemplar_nms_os_threshold),rs.bbs);

if ~isempty(rs.xs)
  for i = 1:length(rs.bbs)
    if ~isempty(rs.xs{i})
      %NOTE: the fifth field must contain elements
      rs.xs{i} = rs.xs{i}(:,rs.bbs{i}(:,5) );
    end
  end
end

function t = get_pyramid(I, sbin, params)
%Extract feature pyramid from variable I (which could be either an image,
%or already a feature pyramid)

if isnumeric(I)
  if (params.detect_add_flip == 1)
    I = flip_image(I);
  else    
    %take unadulterated "aka" un-flipped image
  end
  
  clear t
  t.size = size(I);

  %Compute pyramid
  [t.hog, t.scales] = esvm_pyramid(I, params);
  t.padder = params.detect_pyramid_padding;
  for level = 1:length(t.hog)
    t.hog{level} = padarray(t.hog{level}, [t.padder t.padder 0], 0);
  end
  
  minsizes = cellfun(@(x)min([size(x,1) size(x,2)]), t.hog);
  t.hog = t.hog(minsizes >= t.padder*2);
  t.scales = t.scales(minsizes >= t.padder*2);  
else
  fprintf(1,'Already found features\n');
  
  if iscell(I)
    if params.detect_add_flip==1
      t = I{2};
    else
      t = I{1};
    end
  else
    t = I;
  end
end

