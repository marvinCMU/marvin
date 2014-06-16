function M = mmht_scores(boxes, maxos, models, neighbor_thresh, count_thresh)
%Given a bunch of detections, learn the M boosting matrix, which
%makes the final scores multiplexed

if ~exist('neighbor_thresh','var')
  neighbor_thresh = 0.5;
end

if ~exist('count_thresh','var')
  count_thresh = 0.5;
end

%already nms-ed within exemplars
if 1
for i = 1:length(boxes)
  boxes{i}(:,5) = 1:size(boxes{i},1);
  boxes{i} = nms_within_exemplars(boxes{i},.5);
  maxos{i} = maxos{i}(boxes{i}(:,5));
  boxes{i}(:,5) = i;
end
end

K = length(models);
N = sum(cellfun(@(x)size(x,2),maxos));

y = cat(1,maxos{:});
os = cat(1,maxos{:})';

scores = cellfun2(@(x)x(:,end)',boxes);
scores = [scores{:}];

xraw = cell(length(boxes),1);
allboxes = cat(1,boxes{:});

for i = 1:length(boxes)
  fprintf(1,'.');
  xraw{i} = get_box_features(boxes{i}, K, neighbor_thresh);
end
x = [xraw{:}];

exids = allboxes(:,6);
exids(allboxes(:,7)==1)= exids(allboxes(:,7)==1) + length(models);
imids = allboxes(:,5);

%osmats = cellfun2(@(x)getosmatrix_bb(x,x),boxes);
%thetadiffmats = cellfun2(@(x)getaspectmatrix_bb(x,x),boxes);


fprintf(1,'learning M by counting\n');
tic
%This one works best so far
M = learn_M_counting(x, exids, os, count_thresh);
toc

M.neighbor_thresh = neighbor_thresh;
M.count_thresh = count_thresh;

r = cell(length(xraw),1);
fprintf(1,'applying boost matrix\n');
tic
for j = 1:length(xraw)
  r{j} = apply_boost_M(xraw{j},boxes{j},M);
end

r = [r{:}];
[aa,bb] = sort(r,'descend');
goods = os>.5;
%lenvec = 1:N;

res = (cumsum(goods(bb))./(1:length(bb)));
M.score = mean(res);
toc

figure(4)
subplot(1,2,1)
plot(scores,os,'r.')
xlabel('singleton scores')
ylabel('OS with gt')

subplot(1,2,2)
plot(r,os,'r.')
xlabel('combined score')
ylabel('os')

figure(5)
clf
[aa,bb] = sort(scores,'descend');
plot(cumsum(os(bb)>.5)./(1:length(os)),'r-')
hold on;
[aa,bb] = sort(r,'descend');
plot(cumsum(os(bb)>.5)./(1:length(os)),'b-')
title('Precision-Recall');
legend('singleton','combined')



function M = learn_M_counting(x, exids, os, count_thresh)
% function M = learn_M_counting(x, exids, maxos, count_thresh, osmats, ...
%                               thetadiffmats, boxes)
%Learn the matrix by counting activations on positives
N = size(x,2);
K = size(x,1);

C = zeros(K,K);
%C2 = zeros(K,K);
for i = 1:N
  cur = find(x(:,i)>0);  
  % C(cur,exids(i)) = C(cur,exids(i)) + os(i)*(os(i) >= count_thresh) / ...
  %     length(cur)*sum(x(:,i));
  
  % C2(cur,exids(i)) = C(cur,exids(i)) + (os(i) < count_thresh) / ...
  %     length(cur)*sum(x(:,i));
  coeffs = x(cur,i).*x(exids(i),i);
  C(cur,exids(i)) = C(cur,exids(i)) + coeffs.*double(os(i) >= count_thresh);
  %C2(cur,exids(i)) = C(cur,exids(i)) + coeffs.*double(os(i) < count_thresh);
end

%C = C ./(C2+eps);

for i = 1:K
  M.w{i} = .1 + C(:,i);
  M.b{i} = 0;
end

M.C = sparse(C);
