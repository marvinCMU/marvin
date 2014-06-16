function [resstruct,feat_pyramid] = esvm_detect_rankedList(I, models, params)

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

doflip = params.detect_add_flip;

params.detect_add_flip = 0;

[rs1, t1] = esvm_detectdriver_rankedList(I, models, params);

rs1 = prune_nms(rs1, params);

if doflip == 1
  params.detect_add_flip = 1;
  [rs2, t2] = esvm_detectdriver_rankedList(I, models, params);
  rs2 = prune_nms(rs2, params);
  if params.truncatedModels
    delete(params.ranked_list_filename);
  end
else %If there is no flip, then we are done
  resstruct = rs1;
  feat_pyramid = t1;
  if params.truncatedModels
    delete(params.ranked_list_filename);
  end
  return;
end
%If we got here, then the flip was turned on and we need to concatenate
%results
for q = 1:length(rs1.bbs)
  rs1.xs{q} = cat(2,rs1.xs{q}, ...
                  rs2.xs{q});
  rs1.bbs{q} = cat(1,rs1.bbs{q},rs2.bbs{q});
end
resstruct = rs1;

%Concatenate normal and LR pyramids
feat_pyramid = cat(1,t1,t2);

function [resstruct,t] = esvm_detectdriver_rankedList(I, models,...
                                             params)
if ~isfield(params,'max_models_before_block_method')
  params.max_models_before_block_method = 20;
end

[resstruct,t] = esvm_detectdriverBLOCK_rankedList(I, models, params);


if (length(models) > params.max_models_before_block_method) ...
      || (~isempty(params.nnmode))
else
    for i = 1:length(resstruct.xs)
        for j = 1:size(resstruct.xs{i},2)
            xs{i}{1,j} = resstruct.xs{i}(:,j);
        end
    end
    try
        resstruct.xs = xs;
    catch
    end
end

return;

% if (length(models) > params.max_models_before_block_method) ...
%       || (~isempty(params.nnmode))
% 
%   [resstruct,t] = esvm_detectdriverBLOCK(I, models, ...
%                                          params);
%   return;
% end


N = length(models);
ws = cellfun2(@(x)x.model.w,models);
bs = cellfun2(@(x)x.model.b,models);

%NOTE: all exemplars in this set must have the same sbin
luq = 1;

if isfield(models{1}.model,'init_params')
  sbins = cellfun(@(x)x.model.init_params.sbin,models);
  luq = length(unique(sbins));
end

if isfield(models{1}.model,'init_params') && luq == 1
  sbin = models{1}.model.init_params.sbin;
elseif ~isfield(models{1}.model,'init_params')
  if isfield(params,'init_params')
    sbin = params.init_params.sbin;
  else
    fprintf(1,'No hint for sbin!\n');
    error('No sbin provided');
  end
  
else
  fprintf(1,['Warning: not all exemplars have save sbin, using' ...
             ' first]\n']);
  sbin = models{1}.model.init_params.sbin;
end



t = get_pyramid(I, sbin, params);

resstruct.padder = t.padder;
resstruct.bbs = cell(N,1);
xs = cell(N,1);

maxers = cell(N,1);
for q = 1:N
  maxers{q} = -inf;
end


if params.dfun == 1
  wxs = cellfun2(@(x)reshape(x.model.x(:,1),size(x.model.w)), ...
                 models);
  ws2 = ws;
  special_offset = zeros(length(ws2),1);
  for q = 1:length(ws2)
    ws2{q} = -2*ws{q}.*wxs{q};
    special_offset(q) = ws{q}(:)'*(models{q}.model.x(:,1).^2);
  end
end

%start with smallest level first
for level = length(t.hog):-1:1
  featr = t.hog{level};
  
  if params.dfun == 1
    featr_squared = featr.^2;
    
    %Use blas-based fast convolution code
    rootmatch1 = fconvblas(featr_squared, ws, 1, N);
    rootmatch2 = fconvblas(featr, ws2, 1, N);
     
    for z = 1:length(rootmatch1)
      rootmatch{z} = rootmatch1{z} + rootmatch2{z} + special_offset(z);
    end
    
  else  
    %Use blas-based fast convolution code
    rootmatch = fconvblas(featr, ws, 1, N);
    root_size = cellfun(@(x)size(x),rootmatch,'UniformOutput',false);
    
    if params.usingViewGradients
        for q = 1:N
            for p = 1:size(models{q}.model.DIC,2)
                DIC_template_q{p} = reshape(models{q}.model.DIC(:,p),models{q}.model.hg_size);
            end
            grad_term{q} = fconvblas(featr, DIC_template_q, 1, size(models{q}.model.DIC,2));
            grad_term_sub = cellfun(@(x)vec(x.^2),grad_term{q},'UniformOutput',false);
            grad_term_sub = cat(2,grad_term_sub{:});
            grad_term_sub = sum(grad_term_sub,2);
            grad_term{q} = 1/(4*params.grad_lambda)*reshape(grad_term_sub,root_size{q});
            % make the mean to be 0 which is equivalent of modifying the
            % threshold
%             grad_term{q} = grad_term{q} - mean(grad_term{q}(:));
            rootmatch{q} = rootmatch{q} + grad_term{q};
        end
    end
    
      
   end
    
  
  rmsizes = cellfun2(@(x)size(x), ...
                     rootmatch);
  
  for exid = 1:N
    if prod(rmsizes{exid}) == 0
      continue
    end

    cur_scores = rootmatch{exid} - bs{exid};
    [aa,indexes] = sort(cur_scores(:),'descend');
    NKEEP = sum((aa>maxers{exid}) & (aa>=params.detect_keep_threshold));
    aa = aa(1:NKEEP);
    indexes = indexes(1:NKEEP);
    if NKEEP==0
      continue
    end
    sss = size(ws{exid});
    
    [uus,vvs] = ind2sub(rmsizes{exid}(1:2),...
                        indexes);
    
    scale = t.scales(level);
    
    o = [uus vvs] - t.padder;

    bbs = ([o(:,2) o(:,1) o(:,2)+size(ws{exid},2) ...
               o(:,1)+size(ws{exid},1)] - 1) * ...
             sbin/scale + 1 + repmat([0 0 -1 -1],length(uus),1);

    bbs(:,5:12) = 0;
    bbs(:,5) = (1:size(bbs,1));
    bbs(:,6) = exid;
    bbs(:,8) = scale;
    bbs(:,9) = uus;
    bbs(:,10) = vvs;
    bbs(:,12) = aa;
    
    if (params.detect_add_flip == 1)
      bbs = flip_box(bbs,t.size);
      bbs(:,7) = 1;
    end
    
    resstruct.bbs{exid} = cat(1,resstruct.bbs{exid},bbs);
    
    if params.detect_save_features == 1
      for z = 1:NKEEP
        xs{exid}{end+1} = ...
            reshape(t.hog{level}(uus(z)+(1:sss(1))-1, ...
                                 vvs(z)+(1:sss(2))-1,:), ...
                    [],1);
      end
    end
        
    if (NKEEP > 0)
      newtopk = min(params.detect_max_windows_per_exemplar,size(resstruct.bbs{exid},1));
      [aa,bb] = psort(-resstruct.bbs{exid}(:,end),newtopk);
      resstruct.bbs{exid} = resstruct.bbs{exid}(bb,:);
      if params.detect_save_features == 1
        xs{exid} = xs{exid}(:,bb);
      end
      %TJM: changed so that we only maintain 'maxers' when topk
      %elements are filled
      if (newtopk >= params.detect_max_windows_per_exemplar)
        maxers{exid} = min(-aa);
      end
    end    
  end
end

if params.detect_save_features == 1
  resstruct.xs = xs;
else
  resstruct.xs = cell(N,1);
end
%fprintf(1,'\n');

function [resstruct,t] = esvm_detectdriverBLOCK_rankedList(I, models,...
                                             params)

%%HERE is the chunk version of exemplar localization

N = length(models);
ws = cellfun2(@(x)x.model.w,models);
bs = cellfun(@(x)x.model.b,models)';
bs = reshape(bs,[],1);
sizes1 = cellfun(@(x)x.model.hg_size(1),models);
sizes2 = cellfun(@(x)x.model.hg_size(2),models);

S = [max(sizes1(:)) max(sizes2(:))];
fsize = params.init_params.features();
templates = zeros(S(1),S(2),fsize,length(models));

if isfield(params,'exemplar_matrix') && ~isempty(params.exemplar_matrix)
    exemplar_matrix = params.exemplar_matrix;
else
    for i = 1:length(models)
      t = zeros(S(1),S(2),fsize);
      t(1:models{i}.model.hg_size(1),1:models{i}.model.hg_size(2),:) = ...
          models{i}.model.w;
      templates(:,:,:,i) = t;
    end
    exemplar_matrix = reshape(templates,[],size(templates,4));
end



%maskmat = repmat(template_masks,[1 1 1 fsize]);
%maskmat = permute(maskmat,[1 2 4 3]);
%templates_x  = templates_x .* maskmat;

sbin = models{1}.model.init_params.sbin;

if isfield(params,'precomputation') && ~isempty(params.precomputation)
    if (params.detect_add_flip == 1)
        t = params.precomputation(2);
    else
        t = params.precomputation(1);
    end
else
    t = get_pyramid(I, sbin, params);
end

resstruct.padder = t.padder;

pyr_N = cellfun(@(x)prod([size(x,1) size(x,2)]-S+1),t.hog);
sumN = sum(pyr_N);

X = zeros(S(1)*S(2)*fsize,sumN);
offsets = cell(length(t.hog), 1);
uus = cell(length(t.hog),1);
vvs = cell(length(t.hog),1);

counter = 1;
for i = 1:length(t.hog)
  s = size(t.hog{i});
  NW = s(1)*s(2);
  ppp = reshape(1:NW,s(1),s(2));
  curf = reshape(t.hog{i},[],fsize);
  b = im2col(ppp,[S(1) S(2)]);

  offsets{i} = b(1,:);
  offsets{i}(end+1,:) = i;
  
  for j = 1:size(b,2)
   X(:,counter) = reshape (curf(b(:,j),:),[],1);
   counter = counter + 1;
  end
  
  [uus{i},vvs{i}] = ind2sub(s,offsets{i}(1,:));
end

offsets = cat(2,offsets{:});

uus = cat(2,uus{:});
vvs = cat(2,vvs{:});



if params.truncatedModels
    sleeping_time = 0.1;
    while ~exist(params.ranked_list_filename,'file')
        pause(sleeping_time);
    %     if mod(count,100) == 0
    %         clc;
    %         count = 1;
    %         fprintf('.');
    %     end
    %     fprintf('\n');
    end

    rankedList = load(params.ranked_list_filename);
    val_idx = rankedList>=1 & rankedList<=N;
    rankedList = rankedList(val_idx);
    if ~params.detect_add_flip
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
    end
else
    rankedList = [1:N];
    if ~params.detect_add_flip
        fprintf('non-truncated mode...\n');
    end
end

try
    exemplar_matrix = exemplar_matrix(:,rankedList);
    bs = bs(rankedList);
catch
    error('ranked list contains invalid model index...');
end
    
if params.GPUacc
    Agpu = gpuArray( exemplar_matrix' );
    Bgpu = gpuArray( X );
    % Multiply-add on GPU
    r = ( Agpu * Bgpu);
    % this forces the computation, and converts the result to standard Matlab variable
    r = gather( r );
    r = bsxfun(@minus, r, bs);
else
    r = exemplar_matrix' * X;
    r = bsxfun(@minus, r, bs);
end

resstruct.bbs = cell(N,1);
resstruct.xs = cell(N,1);

for i = 1:length(rankedList)
    exid = rankedList(i);

    goods = find(r(i,:) >= params.detect_keep_threshold);
  
    if isempty(goods)
        continue    
    end

    [sorted_scores,bb] = ...
      psort(-r(i,goods)',...
            min(params.detect_max_windows_per_exemplar, ...
                length(goods)));
    bb = goods(bb);

    sorted_scores = -sorted_scores';

    resstruct.xs{exid} = X(:,bb);

    levels = offsets(2,bb);
    scales = t.scales(levels);
    curuus = uus(bb);
    curvvs = vvs(bb);

    o = [curuus' curvvs'] - t.padder;

    bbs = ([o(:,2) o(:,1) o(:,2)+size(ws{exid},2) ...
           o(:,1)+size(ws{exid},1)] - 1) .* ...
             repmat(sbin./scales',1,4) + 1 + repmat([0 0 -1 ...
                    -1],length(scales),1);

    bbs(:,5:12) = 0;
    bbs(:,5) = (1:size(bbs,1));
    bbs(:,6) = exid;
    bbs(:,8) = scales;
    bbs(:,9) = uus(bb);
    bbs(:,10) = vvs(bb);
    bbs(:,12) = sorted_scores;

    if (params.detect_add_flip == 1)
        bbs = flip_box(bbs,t.size);
        bbs(:,7) = 1;
    end

    resstruct.bbs{exid} = bbs;
end


if params.detect_save_features == 0
  resstruct.xs = cell(N,1);
end
%fprintf(1,'\n');

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
