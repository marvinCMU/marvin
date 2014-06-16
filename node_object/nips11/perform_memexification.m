function [betas,ALL_bboxes] = perform_memexification(grid, models, fg, setname)
% Perform calibration by learning the sigmoid parameters (linear
% transformation of svm scores) for each model independently. If we
% perform an operation such as NMS, we will now have "comparable"
% scores.  This is performed on the 'trainval' set for PASCAL VOC.

% Tomasz Malisiewicz (tomasz@cmu.edu)

%keep things above this threshold for densification
MEMEX_OS_THRESH = 0.5;

%if enabled, do NMS, if disabled return raw detections
DO_NMS = 0;

if DO_NMS == 0
  fprintf(1,'disabled NMS!\n');
end
%only keep detections that have this overlap score with the entire image
OS_THRESH = 0;

if OS_THRESH > 0
  fprintf(1,'WARNING: only keeping detections above OS threshold: %.3f\n',...
          OS_THRESH);
end

%targetcls = 'train';
targetcls = 'sheep';

if ~exist('models','var')
  models = load_all_exemplars(targetcls);
end

if ~exist('grid','var')
  grid = load_result_grid(models);
end

if ~exist('fg','var')
  fg = cat(1,get_pascal_bg('trainval'),get_pascal_bg('test'));
end

if ~exist('setname','var')
  setname = 'voc';
end

% if enabled, display images
display = 0;

% if display is enabled and dump_images is enabled, then dump images into DUMPDIR
dump_images = 0;

VOCinit;
DUMPDIR = sprintf('%s/%s/%s/',VOCopts.dumpdir,VOCopts.dataset,models{1}.cls);
if ~exist(DUMPDIR,'dir')
  mkdir(DUMPDIR);
end
% show A NxN grid of top detections (if display is turned on)
SHOW_TOP_N_SVS = 10;

%if nargin < 1
%  fprintf(1,'Not enough arguments, need at least the grid\n');
%  return;
%end

final_dir = ...
    sprintf('%s/betas',VOCopts.localdir);

% if ~exist('fg','var')
%   fprintf(1,'Loading default set of images\n');
%   %[bg,setname] = default_testset;
%   fg = eval(models{1}.fg);
  
%   %fprintf(1,'HACK USING RAND BG\n');
%   %fg = get_james_bg(100000,round(linspace(1,6400000,100000)));
%   setname = 'sketchfg';
% end

bg_size = length(eval(models{1}.bg));

if strcmp(setname,'voc')
  target_directory = 'trainval';
  fprintf(1,'Using VOC set so performing calibration with set: %s\n',target_directory);
  %% prune grid to contain only images from target_directory
  [cur_set,gt] = textread(sprintf(VOCopts.imgsetpath,target_directory),['%s' ...
                    ' %d']);

  for i = 1:length(grid)
    [tmp,curid,tmp] = fileparts(fg{grid{i}.index});
    grid{i}.curid = curid;
  end
  
  gridids = cellfun2(@(x)x.curid,grid);
  goods = ismember(gridids,cur_set);
  grid = grid(goods);
  
end

if ~exist(final_dir','dir')
  mkdir(final_dir);
end

final_file = ...
    sprintf('%s/betas/%s_betas.mat',...
            VOCopts.localdir,models{1}.cls);

CACHE_BETAS = 0;
if CACHE_BETAS == 1 && fileexists(final_file)
  %fprintf(1,'not loading!!!!!\n')
  %display = 1;
  fprintf(1,'Loading final file %s\n',final_file);
  load(final_file);
  return;
end

for i = 1:length(models)
  if ~isfield(models{i},'curid')
    models{i}.curid = '-1';
  end
end

model_ids = cellfun2(@(x)x.curid,models);

targets = 1:length(models);

cls = models{1}.cls;

for i = 1:length(grid)    
  if mod(i,100)==0
    fprintf(1,'.');
  end
  cur = grid{i};
  %cur.bboxes = cur.coarse_boxes;
  % Do image-OS pruning, BEFORE NMS
  % this is useful if we are in image detection mode, where we want
  % to retain detections that are close to the entire image
  if OS_THRESH > 0
    curos = getosmatrix_bb(cur.bboxes(:,1:4),cur.imbb);
    cur.bboxes = cur.bboxes(curos>=OS_THRESH,:);
  end
  
  if size(cur.bboxes,1) >= 1
    cur.bboxes(:,5) = 1:size(cur.bboxes,1);    
    if DO_NMS == 1
      cur.bboxes = nms_within_exemplars(cur.bboxes,.5);
    end
    if length(cur.extras)>0
      cur.extras.os = cur.extras.os(cur.bboxes(:,5),:);
    end
  end
  
  cur.bboxes(:,5) = grid{i}.index;
  
  bboxes{i} = cur.bboxes;
   
  %if we have overlaps, collect them
  if length(cur.extras) > 0
    
    %use all objects as ground truth
    %goods = 1:length(cur.extras.cats);
    

    %% find the ground truth examples of the right category
    goods = find(ismember(cur.extras.cats,cls));
    
    exids = cur.bboxes(:,6);
   
    if length(goods) == 0
      os{i} = zeros(size(bboxes{i},1),1);
    else
      curos = cur.extras.os(:,goods);
      os{i} = max(curos,[],2);
    end    
  else
    os{i} = zeros(size(bboxes{i},1),1);    
  end
  
  scores{i} = cur.bboxes(:,7)';
end
  
ALL_bboxes = cat(1,bboxes{:});
ALL_os = cat(1,os{:});


if nargout == 2
  betas = [];
  return;
end
%[aa, bb] = sort(ALL_bboxes(:,end), 'descend');

curids = cellfun2(@(x)x.curid,grid);
for exid = 1:length(models)
  fprintf(1,'.');
  

  filer = sprintf('%s/exemplars/%s.%d.memex-%s.mat',...
                  VOCopts.localdir,...
                  models{exid}.curid,...
                  models{exid}.objectid,...
                  models{exid}.cls);
  filer
  
  filerlock = [filer '.lock'];
  
  if fileexists(filer) || (mymkdir_dist(filerlock) == 0)
    continue
  end
  sourcegrid = find(ismember(curids,models{exid}.curid));
  if length(sourcegrid) == 0
    sourcegrid = -1;
  end
  %REMOVE SOURCE IMAGE TOO
  %HACK removed
  hits = find((ALL_bboxes(:,6)==exid));%% & (ALL_bboxes(:,5) ~= sourcegrid));
  all_scores = ALL_bboxes(hits,end);
  all_os = ALL_os(hits,:);
  
  good_scores = all_scores(all_os>=.5);
  good_os = all_os(all_os>=.5);
  
  bad_scores = all_scores(all_os<.5);
  bad_os = all_os(all_os<.5);

  %add virtual sample at os=1.0, score=1.0
  good_os = cat(1,good_os,1.0);
  good_scores = cat(1,good_scores,1.0);

  if length(good_os) <= 1
    beta = [.1 100];
  else

    [aa,bb] = sort(bad_scores,'descend');
    curlen = min(length(bb),10000*length(good_scores));
    bb = bb(round(linspace(1,length(bb),curlen)));

    bad_scores = bad_scores(bb);
    bad_os = bad_os(bb);
    all_scores = [good_scores; bad_scores];
    all_os = [good_os; bad_os];
    
    beta = learn_sigmoid(all_scores, all_os);
  end

  
  %if beta(1)<.001
  %  beta(1) = .001;
  %end
  
  betas(exid,:) = beta;

  if (sum(ismember(exid,targets))==0)
    continue
  end
  
      
  %if exid==221
  %if exid==171
  if 0
    figure(1)
    clf
    subplot(1,2,1)  
    plot(all_scores,all_os,'r.')
    xs = linspace(min(all_scores),max(all_scores),1000);
    fx = @(x)(1./(1+exp(-beta(1)*(x-beta(2)))));
    
    hold on
    plot(xs,fx(xs),'b','LineWidth',2)
    axis([min(xs) max(xs) 0 1])
    xlabel('SVM score')
    ylabel(sprintf('Max Overlap Score with %s',models{exid}.cls))
    
    title(sprintf('Learned Sigmoid \\beta=[%.3f %.3f]',beta(1), ...
                  beta(2)))
    subplot(1,2,2)
    %subplot(2,2,1)
    if isfield(models{exid},'I')
      Iex = im2double(models{exid}.I);  
    else
      %try pascal VOC image
      Iex = im2double(imread(sprintf(VOCopts.imgpath, ...
                                     models{exid}.curid)));
    end
    imagesc(Iex)
    plot_bbox(models{exid}.gt_box)
    %[aa,bb] = max(all_scores);
    %plot_bbox(ALL_bboxes(hits(bb),1:4),'',[0 1 0]);
    axis image
    axis off
    pause
  end
  
  %if (display == 0)
  %  continue
  %end

  
  %subplot(2,2,1)
  if isfield(models{exid},'I')
    Iex = im2double(models{exid}.I);  
  else
    %try pascal VOC image
    Iex = im2double(imread(sprintf(VOCopts.imgpath, ...
                                   models{exid}.curid)));
  end
  
  
  %bbox = models{exid}.model.coarse_box(13,:);
  bbox = models{exid}.gt_box; %model.coarse_box(13,:);
  Iex = pad_image(Iex,300);
  bbox = bbox+300;
  bbox = round(bbox);
  Iex = Iex(bbox(2):bbox(4),bbox(1):bbox(3),:);
  
  show1 = Iex;
  %imagesc(Iex)

  %axis image
  %axis off
  %title(sprintf('Exemplar %d',exid))

  %subplot(2,2,3)
  

  hogpic = (HOGpicture(models{exid}.model.w));
  
  NC = 200;
  colorsheet = jet(NC);
  dists = hogpic(:);    
  dists = dists - min(dists);
  dists = dists / (max(dists)+eps);
  dists = round(dists*(NC-1)+1);
  colors = colorsheet(dists,:);
  show2 = reshape(colors,[size(hogpic,1) size(hogpic,2) 3]);

  %axis image
  %axis off
  %title('Learned Template')
  %drawnow
    
  all_bb = ALL_bboxes(hits,:);
  all_os = ALL_os(hits);
  scores = all_bb(:,end) + all_os;
  %scores(all_os < MEMEX_OS_THRESH) = -100;
  
  
  [alpha,beta] = sort(scores,'descend');
  K = sum(alpha>=-1.0);
  beta = beta(1:K);



  NNN = max(1,ceil(sqrt(length(beta))));


  clear III
  clear IIIscores
  III{1} = zeros(100,100,3);
  IIIscores(1) = -10;
  for aaa = 1:NNN*NNN
    III{aaa} = zeros(100,100,3);
    IIIscores(aaa) = -10;
  end
  
  
  for aaa = 1:NNN*NNN
    fprintf(1,'.');
    if aaa > length(beta)
      break
    end
    
    curI = convert_to_I(fg{all_bb(beta(aaa),5)});   
    %curI = imread(sprintf(VOCopts.imgpath,sprintf('%06d', ...
    %                                              all_bb(beta(aaa),5))));
    %curI = im2double(curI);
    
    bbox = all_bb(beta(aaa),:);
 
    curI = pad_image(curI,300);
    bbox = bbox+300;
    bbox = round(bbox);

    %figure(1)
    %imagesc(curI)
    %drawnow
    try
      Iex = curI(bbox(2):bbox(4),bbox(1):bbox(3),:);
    catch
      Iex = rand(100,100,3);
    end
    Iex = max(0.0,min(1.0,Iex));
    III{aaa} = Iex;
    IIIscores(aaa) = all_bb(beta(aaa),end);
  end
  
  sss = cellfun2(@(x)size(x),III(1:max(1,K)));
  meansize = round(mean(cat(1,sss{:}),1));

  III = cellfun2(@(x)min(1.0,max(0.0,imresize(x,meansize(1:2)))), ...
                 III);  
  
  
  IIIstack = cat(4,III{:});
  IIImean = mean(IIIstack,4);
  
  III2 = cell(1,length(III)+3);

  III2(4:end) = III;

  III2{1} = imresize(show1,meansize(1:2));
  III2{2} = imresize(show2,meansize(1:2));
  III2{3} = IIImean;
  III = III2(1:(end-3));
  
  clear Irow
  III = reshape(III,[NNN NNN]);
  for i = 1:NNN
    Irow{i} = cat(1,III{i,:}); 
  end
  
  I = cat(2,Irow{:});
  figure(1)
  clf
  I = max(0.0,min(1.0,I));
  imagesc(I)
  axis image
  axis off
  [ws1,ws2,ws3] = size(models{exid}.model.w);
  fg_size = length(fg);

  if length(IIIscores)<3
    IIIscores(end+1:3)= -1.1;
  end
  title(sprintf('Wsize=[%d,%d,%d] sbin=%d os=%.3f {|fg|,|bg|}={%d,%d} Top 3 scores (%.3f %.3f %.3f)',ws1,ws2,ws3,...
                models{exid}.model.params.sbin,OS_THRESH,fg_size,bg_size,...
                IIIscores(1),IIIscores(2),IIIscores(3)));
  drawnow
  
  
  if 1
  uims = unique(all_bb(beta,5));
  %% now localize and get all hits

  %x2 are the os-based relevance feedback
  
  x2 = cell(0,1);
  friend_info = cell(0,1);
  for q = 1:length(uims)
    fprintf(1,'x');
    curI = convert_to_I(fg{uims(q)});
    starter=tic;
    [rs,t] = localizemeHOG(curI,models{exid}.model.params.sbin,...
                           {models{exid}.model.w}, ...
                           {models{exid}.model.b},...
                           -1.0,10,10,1);

    scores = cat(2,rs.score_grid{:});
    [aa,bb] = max(scores);
    fprintf(1,' took %.3fsec, maxhit=%.3f, #hits=%d\n',...
            toc(starter),aa,length(scores));
 
    [boxes] = extract_bbs_from_rs(rs,curI,models{1}.model.params.sbin,models(exid));
    %boxes = adjust_boxes(boxes, models);
    curx = cat(2,rs.support_grid{1}{:});
    recs = PASreadrecord(sprintf(VOCopts.annopath, ...
                                 grid{uims(q)}.curid));
    
    %find the objects inside this file that are of the same class
    good_objects = find(ismember({recs.objects.class}, ...
                                 models{1}.cls));
    
    allbbs = cat(1,recs.objects.bbox);
    gtbbs = allbbs(good_objects,:);

    osmat = getosmatrix_bb(boxes,gtbbs);
    [aaa,bbb] = max(osmat,[],2);
    bbb = good_objects(bbb);
    
    %Keep only detections above MEMEX_OS_THRESHOLD
    keepers = find(aaa >= MEMEX_OS_THRESH);
    aaa = aaa(keepers);
    bbb = bbb(keepers);

    
    boxes = boxes(keepers,:);
    curx = curx(:,keepers);
    clear stat
    stat.curid = grid{uims(q)}.curid;
    stat.os = aaa;
    stat.os_id = bbb;
    stat.boxes = boxes;
    stat.allbbs = allbbs;
    
    if 0
      %SHOW the detections in this image
      figure(2)
      clf
      imagesc(curI)
      plot_bbox(boxes);
      title(sprintf('%d boxes',size(boxes,1)))
      drawnow
      pause
    end
    
    if size(boxes,1) == 0
      %% SOMETIMES WE DONT GET THE DETECTION BACK
      continue
    end
    
    x2{end+1} = curx;
    friend_info{end+1} = stat;
  end
  fprintf(1,'\n');

  fprintf(1,'doner\n');
  %get max score in each image

  max_score = cellfun(@(x)max(x.boxes(:,end)),friend_info);

  [aa,bb] = sort(max_score,'descend');
  x2 = x2(bb);
  friend_info = friend_info(bb);
  
  x2 = cat(2,x2{:});
  
  %take old model first, then adjust fields
  m = models{exid};
  
  m.model.x = x2;
  m.friend_info = friend_info;
  
  if length(x2) > 0
    m.model.w = reshape(mean(x2,2),size(m.model.w));
  end
  
  m.model.w = m.model.w - mean(m.model.w(:));
  m.model.b = -10;
  m.models_name = 'iccv_memex';
  mining_queue = '';
  
  NFRIENDS = sum(cellfun(@(x)size(x.boxes,1),m.friend_info));
  fprintf(1,'GOT %d friends\n',NFRIENDS);
  
  m.model.coarse_box = [];
  m.model.svids = [];
  m.model.wtrace = [];
  
  save(filer,'m','mining_queue');
  
  try
    rmdir(filerlock);
  catch
    fprintf(1,'Directory %s already gone\n',filerlock);
  end
  
  end
  
  if dump_images == 1
    figure(1)
    filer = sprintf('%s/result.%d.%s.%s.png', DUMPDIR, ...
                    exid,models{exid}.cls,models{exid}.models_name);
    set(gcf,'PaperPosition',[0 0 20 20]);
    print(gcf,filer,'-dpng');
    
  else
    %pause
  end  
end

if CACHE_BETAS == 1
  fprintf(1,'Loaded betas, saving to %s\n',final_file);
  save(final_file,'betas');
end
