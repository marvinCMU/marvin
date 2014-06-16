function [results] = evaluate_pascal_voc_grid(VOCopts,models,grid, ...
                                              target_directory, ...
                                              final, CACHE_FILE)
%% Evaluate PASCAL VOC detection task with the models, their output
%% firings grid, on the set target_directory which can be either
%% 'trainval' or 'test'

%In case we want to evaluate a subset of detectors
% targets = [4 5];
% for i = 1:length(grid)
%   goods = find(ismember(grid{i}.bboxes(:,6),targets));
%   grid{i}.coarse_boxes = grid{i}.coarse_boxes(goods,:);
%   grid{i}.bboxes = grid{i}.bboxes(goods,:);
%   grid{i}.extras.os = grid{i}.extras.os(goods,:);
% end

if ~exist('CACHE_FILE','var')
  CACHE_FILE = 0;
end
VOCopts.testset = target_directory;

resfile = sprintf('%s/%s.%s%s_%s_results.mat',VOCopts.resdir,...
                  models{1}.models_name,...
                  models{1}.cls,final.calib_string,...
                  target_directory');

if CACHE_FILE && fileexists(resfile) 
  fprintf(1,'Pre loading %s\n',resfile);
  load(resfile);
  return;
end

cls = models{1}.cls;

mname = sprintf('%s%s',models{1}.models_name,final.calib_string);
filer = sprintf('%s/%s/comp3_det_%s_%s.txt',...
                VOCopts.resdir,mname,...
                target_directory,cls);

%Create directory if it is not present
[aaa,bbb,ccc] = fileparts(filer);
if ~exist(aaa,'dir')
  mkdir(aaa);
end

fprintf(1,'Writing File %s\n',filer);
filerlock = [filer '.lock'];
if fileexists(filer) || (mymkdir_dist(filerlock)==0)
  
else
  
  fid = fopen(filer,'w');
  for i = 1:length(final.final_boxes)
    curid = grid{i}.curid;
    for q = 1:size(final.final_boxes{i},1)
      fprintf(fid,'%s %f %f %f %f %f\n',curid,...
              final.final_boxes{i}(q,end),...
              final.final_boxes{i}(q,1:4));
    end
  end
  fclose(fid);
  rmdir(filerlock);
end

%make sure filer is present in order to continue here
wait_until_all_present({filer});

%fprintf(1,'HACK: changing OVERLAP HERE!\n');
%VOCopts.minoverlap = .4;

figure(2)
clf
VOCopts.filename = filer;
[results.recall,results.prec,results.ap,results.apold,results.fp,results.tp,results.npos,results.corr] = VOCevaldet(VOCopts,'comp3',cls,true);

set(gca,'FontSize',16)
set(get(gca,'Title'),'FontSize',16)
set(get(gca,'YLabel'),'FontSize',16)
set(get(gca,'XLabel'),'FontSize',16)
axis([0 1 0 1]);

filer = sprintf(['%s/www/%s-%s%s-on-%s.pdf'], ...
                VOCopts.localdir, ...
                models{1}.cls,...
                models{1}.models_name,...
                final.calib_string, ...
                target_directory);
[basedir,tmp,tmp] = fileparts(filer);
if ~exist(basedir,'dir')
  mkdir(basedir);
end
set(gcf,'PaperPosition',[0 0 8 8])
print(gcf,'-dpdf',filer);
filer2 = strrep(filer,'.pdf','.png');
print(gcf,'-dpng',filer2);

fprintf(1,'Just Wrote %s\n',filer);

results.cls = models{1}.cls;
drawnow

%TODO: we are saving really large files for exemplarNN
save(resfile,'results','final');
