function exemplar_matrix = gen_exemplar_matrix(models,params)
sizes1 = cellfun(@(x)x.model.hg_size(1),models);
sizes2 = cellfun(@(x)x.model.hg_size(2),models);

S = [max(sizes1(:)) max(sizes2(:))];
fsize = params.init_params.features();
templates = zeros(S(1),S(2),fsize,length(models));

for i = 1:length(models)
  t = zeros(S(1),S(2),fsize);
  t(1:models{i}.model.hg_size(1),1:models{i}.model.hg_size(2),:) = ...
      models{i}.model.w;
  templates(:,:,:,i) = t;
end
exemplar_matrix = reshape(templates,[],size(templates,4));