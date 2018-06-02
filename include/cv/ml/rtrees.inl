/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/



CvForestTree::CvForestTree()
{
    forest = NULL;
}


CvForestTree::~CvForestTree()
{
    clear();
}


bool CvForestTree::train(CvDTreeTrainData* _data,
                          const img_t* _subsample_idx,
                          CvRTrees* _forest)
{
    clear();
    forest = _forest;

    data = _data;
    data->shared = true;
    return do_train(_subsample_idx);
}


bool
CvForestTree::train(const img_t*, int, const img_t*, const img_t*,
                    const img_t*, const img_t*, const img_t*, CvDTreeParams)
{
    assert(0);
    return false;
}


bool
CvForestTree::train(CvDTreeTrainData*, const img_t*)
{
    assert(0);
    return false;
}



namespace cv
{

ForestTreeBestSplitFinder::ForestTreeBestSplitFinder(CvForestTree* _tree, CvDTreeNode* _node) :
    DTreeBestSplitFinder(_tree, _node) {}

ForestTreeBestSplitFinder::ForestTreeBestSplitFinder(const ForestTreeBestSplitFinder& finder, Split spl) :
    DTreeBestSplitFinder(finder, spl) {}

void ForestTreeBestSplitFinder::operator()(const BlockedRange& range)
{
    int vi, vi1 = range.begin(), vi2 = range.end();
    int n = node->sample_count;
    CvDTreeTrainData* data = tree->get_data();
    CAutoBuffer<uchar> inn_buf(2*n*(sizeof(int) + sizeof(float)));

    CvForestTree* ftree = (CvForestTree*)tree;
    const img_t* active_var_mask = ftree->forest->get_active_var_mask();

    for(vi = vi1; vi < vi2; vi++)
    {
        CvDTreeSplit *res;
        int ci = data->var_type->tt.i[vi];
        if(node->num_valid[vi] <= 1
            || (active_var_mask && !active_var_mask->tt.data[vi]))
            continue;

        if(data->is_classifier)
        {
            if(ci >= 0)
                res = ftree->find_split_cat_class(node, vi, bestSplit->quality, split, (uchar*)inn_buf);
            else
                res = ftree->find_split_ord_class(node, vi, bestSplit->quality, split, (uchar*)inn_buf);
        }
        else
        {
            if(ci >= 0)
                res = ftree->find_split_cat_reg(node, vi, bestSplit->quality, split, (uchar*)inn_buf);
            else
                res = ftree->find_split_ord_reg(node, vi, bestSplit->quality, split, (uchar*)inn_buf);
        }

        if(res && bestSplit->quality < split->quality)
                memcpy((CvDTreeSplit*)bestSplit, (CvDTreeSplit*)split, splitSize);
    }
}
}

CvDTreeSplit* CvForestTree::find_best_split(CvDTreeNode* node)
{
    img_t* active_var_mask = 0;
    if(forest)
    {
        int var_count;
        CRNG* rng = forest->get_rng();

        active_var_mask = forest->get_active_var_mask();
        var_count = active_var_mask->cols;

        CC_Assert(var_count == data->var_count);

        for(int vi = 0; vi < var_count; vi++)
        {
            uchar temp;
            int i1 = cvRandInt(rng) % var_count;
            int i2 = cvRandInt(rng) % var_count;
            CC_SWAP(active_var_mask->tt.data[i1],
                active_var_mask->tt.data[i2], temp);
        }
    }

    ForestTreeBestSplitFinder finder(this, node);

    parallel_reduce(BlockedRange(0, data->var_count), finder);

    CvDTreeSplit *bestSplit = 0;
    if(finder.bestSplit->quality > 0)
    {
        bestSplit = data->new_split_cat(0, -1.0f);
        memcpy(bestSplit, finder.bestSplit, finder.splitSize);
    }

    return bestSplit;
}

void CvForestTree::read(CvFileStorage* fs, CvFileNode* fnode, CvRTrees* _forest, CvDTreeTrainData* _data)
{
    CvDTree::read(fs, fnode, _data);
    forest = _forest;
}


void CvForestTree::read(CvFileStorage*, CvFileNode*)
{
    assert(0);
}

void CvForestTree::read(CvFileStorage* _fs, CvFileNode* _node,
                         CvDTreeTrainData* _data)
{
    CvDTree::read(_fs, _node, _data);
}


//////////////////////////////////////////////////////////////////////////////////////////
//                                  Random trees                                        //
//////////////////////////////////////////////////////////////////////////////////////////
CvRTParams::CvRTParams() : CvDTreeParams(5, 10, 0, false, 10, 0, false, false, 0),
    calc_var_importance(false), nactive_vars(0)
{
    term_crit = cTermCriteria(CC_TERMCRIT_ITER+CC_TERMCRIT_EPS, 50, 0.1);
}

CvRTParams::CvRTParams(int _max_type, int _min_sample_count,
                        float _regression_accuracy, bool _use_surrogates,
                        int _max_categories, const float* _priors, bool _calc_var_importance,
                        int _nactive_vars, int max_num_of_trees_in_the_forest,
                        float forest_accuracy, int termcrit_type) :
    CvDTreeParams(_max_type, _min_sample_count, _regression_accuracy,
                   _use_surrogates, _max_categories, 0,
                   false, false, _priors),
    calc_var_importance(_calc_var_importance),
    nactive_vars(_nactive_vars)
{
    term_crit = cTermCriteria(termcrit_type,
        max_num_of_trees_in_the_forest, forest_accuracy);
}

CvRTrees::CvRTrees()
{
    nclasses         = 0;
    oob_error        = 0;
    ntrees           = 0;
    trees            = NULL;
    data             = NULL;
    active_var_mask  = NULL;
    var_importance   = NULL;
    rng = &theRNG();
    default_model_name = "my_random_trees";
}


void CvRTrees::clear()
{
    int k;
    for(k = 0; k < ntrees; k++)
        delete trees[k];
    cFree(&trees);

    delete data;
    data = 0;

    cvReleaseMat(&active_var_mask);
    cvReleaseMat(&var_importance);
    ntrees = 0;
}


CvRTrees::~CvRTrees()
{
    clear();
}

std::string CvRTrees::getName() const
{
    return CC_TYPE_NAME_ML_RTREES;
}

img_t* CvRTrees::get_active_var_mask()
{
    return active_var_mask;
}


CRNG* CvRTrees::get_rng()
{
    return &rng->state;
}

bool CvRTrees::train(const img_t* _train_data, int _tflag,
                        const img_t* _responses, const img_t* _var_idx,
                        const img_t* _sample_idx, const img_t* _var_type,
                        const img_t* _missing_mask, CvRTParams params)
{
    clear();

    CvDTreeParams tree_params(params.max_type, params.min_sample_count,
        params.regression_accuracy, params.use_surrogates, params.max_categories,
        params.cv_folds, params.use_1se_rule, false, params.priors);

    data = new CvDTreeTrainData();
    data->set_data(_train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, tree_params, true);

    int var_count = data->var_count;
    if(params.nactive_vars > var_count)
        params.nactive_vars = var_count;
    else if(params.nactive_vars == 0)
        params.nactive_vars = (int)sqrt((double)var_count);
    else if(params.nactive_vars < 0)
        CC_Error(CC_StsBadArg, "<nactive_vars> must be non-negative");

    // Create mask of active variables at the tree nodes
    active_var_mask = cvCreateMat(1, var_count, CC_8UC1);
    if(params.calc_var_importance)
    {
        var_importance  = cvCreateMat(1, var_count, CC_32FC1);
        cvZero(var_importance);
    }
    { // initialize active variables mask
        img_t submask1, submask2;
        CC_Assert((active_var_mask->cols >= 1) && (params.nactive_vars > 0) && (params.nactive_vars <= active_var_mask->cols));
        cvGetCols(active_var_mask, &submask1, 0, params.nactive_vars);
        cvSet(&submask1, cScalar(1));
        if(params.nactive_vars < active_var_mask->cols)
        {
            cvGetCols(active_var_mask, &submask2, params.nactive_vars, var_count);
            cvZero(&submask2);
        }
    }

    return grow_forest(params.term_crit);
}

bool CvRTrees::train(CvMLData* _data, CvRTParams params)
{
    const img_t* values = _data->get_values();
    const img_t* response = _data->get_responses();
    const img_t* missing = _data->get_missing();
    const img_t* var_types = _data->get_var_types();
    const img_t* train_sidx = _data->get_train_sample_idx();
    const img_t* var_idx = _data->get_var_idx();

    return train(values, CC_ROW_SAMPLE, response, var_idx,
                  train_sidx, var_types, missing, params);
}

bool CvRTrees::grow_forest(const CTermCriteria term_crit)
{
    img_t* sample_idx_mask_for_tree = 0;
    img_t* sample_idx_for_tree      = 0;

    const int max_ntrees = term_crit.max_iter;
    const double max_oob_err = term_crit.epsilon;

    const int dims = data->var_count;
    float maximal_response = 0;

    img_t* oob_sample_votes    = 0;
    img_t* oob_responses       = 0;

    float* oob_samples_perm_ptr= 0;

    float* samples_ptr     = 0;
    uchar* missing_ptr     = 0;
    float* true_resp_ptr   = 0;
    bool is_oob_or_vimportance = (max_oob_err > 0 && term_crit.type != CC_TERMCRIT_ITER) || var_importance;

    // oob_predictions_sum[i] = sum of predicted values for the i-th sample
    // oob_num_of_predictions[i] = number of summands
    //                            (number of predictions for the i-th sample)
    // initialize these variable to avoid warning C4701
    img_t oob_predictions_sum = cvMat(1, 1, CC_32FC1);
    img_t oob_num_of_predictions = cvMat(1, 1, CC_32FC1);

    nsamples = data->sample_count;
    nclasses = data->get_num_classes();

    if (is_oob_or_vimportance)
    {
        if(data->is_classifier)
        {
            oob_sample_votes = cvCreateMat(nsamples, nclasses, CC_32SC1);
            cvZero(oob_sample_votes);
        }
        else
        {
            // oob_responses[0,i] = oob_predictions_sum[i]
            //    = sum of predicted values for the i-th sample
            // oob_responses[1,i] = oob_num_of_predictions[i]
            //    = number of summands (number of predictions for the i-th sample)
            oob_responses = cvCreateMat(2, nsamples, CC_32FC1);
            cvZero(oob_responses);
            cvGetRow(oob_responses, &oob_predictions_sum, 0);
            cvGetRow(oob_responses, &oob_num_of_predictions, 1);
        }

        oob_samples_perm_ptr     = (float*)cAlloc(sizeof(float)*nsamples*dims);
        samples_ptr              = (float*)cAlloc(sizeof(float)*nsamples*dims);
        missing_ptr              = (uchar*)cAlloc(sizeof(uchar)*nsamples*dims);
        true_resp_ptr            = (float*)cAlloc(sizeof(float)*nsamples);

        data->get_vectors(0, samples_ptr, missing_ptr, true_resp_ptr);

        double minval, maxval;
        img_t responses = cvMat(1, nsamples, CC_32FC1, true_resp_ptr);
        cvMinMaxLoc(&responses, &minval, &maxval);
        maximal_response = (float)MAX(MAX(fabs(minval), fabs(maxval)), 0);
    }

    trees = (CvForestTree**)cAlloc(sizeof(trees[0])*max_ntrees);
    memset(trees, 0, sizeof(trees[0])*max_ntrees);

    sample_idx_mask_for_tree = cvCreateMat(1, nsamples, CC_8UC1);
    sample_idx_for_tree      = cvCreateMat(1, nsamples, CC_32SC1);

    ntrees = 0;
    while(ntrees < max_ntrees)
    {
        int i, oob_samples_count = 0;
        double ncorrect_responses = 0; // used for estimation of variable importance
        CvForestTree* tree = 0;

        cvZero(sample_idx_mask_for_tree);
        for(i = 0; i < nsamples; i++) //form sample for creation one tree
        {
            int idx = (*rng)(nsamples);
            sample_idx_for_tree->tt.i[i] = idx;
            sample_idx_mask_for_tree->tt.data[idx] = 0xFF;
        }

        trees[ntrees] = new CvForestTree();
        tree = trees[ntrees];
        tree->train(data, sample_idx_for_tree, this);

        if (is_oob_or_vimportance)
        {
            img_t sample, missing;
            // form array of OOB samples indices and get these samples
            sample   = cvMat(1, dims, CC_32FC1, samples_ptr);
            missing  = cvMat(1, dims, CC_8UC1,  missing_ptr);

            oob_error = 0;
            for(i = 0; i < nsamples; i++,
                sample->tt.fl += dims, missing->tt.data += dims)
            {
                CvDTreeNode* predicted_node = 0;
                // check if the sample is OOB
                if(sample_idx_mask_for_tree->tt.data[i])
                    continue;

                // predict oob samples
                if(!predicted_node)
                    predicted_node = tree->predict(&sample, &missing, true);

                if(!data->is_classifier) //regression
                {
                    double avg_resp, resp = predicted_node->value;
                    oob_predictions_sum->tt.fl[i] += (float)resp;
                    oob_num_of_predictions->tt.fl[i] += 1;

                    // compute oob error
                    avg_resp = oob_predictions_sum->tt.fl[i]/oob_num_of_predictions->tt.fl[i];
                    avg_resp -= true_resp_ptr[i];
                    oob_error += avg_resp*avg_resp;
                    resp = (resp - true_resp_ptr[i])/maximal_response;
                    ncorrect_responses += exp(-resp*resp);
                }
                else //classification
                {
                    double prdct_resp;
                    CPoint max_loc;
                    img_t votes;

                    cvGetRow(oob_sample_votes, &votes, i);
                    votes->tt.i[predicted_node->class_idx]++;

                    // compute oob error
                    cvMinMaxLoc(&votes, 0, 0, 0, &max_loc);

                    prdct_resp = data->cat_map->tt.i[max_loc.x];
                    oob_error += (fabs(prdct_resp - true_resp_ptr[i]) < FLT_EPSILON) ? 0 : 1;

                    ncorrect_responses += cRound(predicted_node->value - true_resp_ptr[i]) == 0;
                }
                oob_samples_count++;
            }
            if(oob_samples_count > 0)
                oob_error /= (double)oob_samples_count;

            // estimate variable importance
            if(var_importance && oob_samples_count > 0)
            {
                int m;

                memcpy(oob_samples_perm_ptr, samples_ptr, dims*nsamples*sizeof(float));
                for(m = 0; m < dims; m++)
                {
                    double ncorrect_responses_permuted = 0;
                    // randomly permute values of the m-th variable in the oob samples
                    float* mth_var_ptr = oob_samples_perm_ptr + m;

                    for(i = 0; i < nsamples; i++)
                    {
                        int i1, i2;
                        float temp;

                        if(sample_idx_mask_for_tree->tt.data[i]) //the sample is not OOB
                            continue;
                        i1 = (*rng)(nsamples);
                        i2 = (*rng)(nsamples);
                        CC_SWAP(mth_var_ptr[i1*dims], mth_var_ptr[i2*dims], temp);

                        // turn values of (m-1)-th variable, that were permuted
                        // at the previous iteration, untouched
                        if(m > 1)
                            oob_samples_perm_ptr[i*dims+m-1] = samples_ptr[i*dims+m-1];
                    }

                    // predict "permuted" cases and calculate the number of votes for the
                    // correct class in the variable-m-permuted oob data
                    sample  = cvMat(1, dims, CC_32FC1, oob_samples_perm_ptr);
                    missing = cvMat(1, dims, CC_8UC1, missing_ptr);
                    for(i = 0; i < nsamples; i++,
                        sample->tt.fl += dims, missing->tt.data += dims)
                    {
                        double predct_resp, true_resp;

                        if(sample_idx_mask_for_tree->tt.data[i]) //the sample is not OOB
                            continue;

                        predct_resp = tree->predict(&sample, &missing, true)->value;
                        true_resp   = true_resp_ptr[i];
                        if(data->is_classifier)
                            ncorrect_responses_permuted += cRound(true_resp - predct_resp) == 0;
                        else
                        {
                            true_resp = (true_resp - predct_resp)/maximal_response;
                            ncorrect_responses_permuted += exp(-true_resp*true_resp);
                        }
                    }
                    var_importance->tt.fl[m] += (float)(ncorrect_responses
                        - ncorrect_responses_permuted);
                }
            }
        }
        ntrees++;
        if(term_crit.type != CC_TERMCRIT_ITER && oob_error < max_oob_err)
            break;
    }

    if(var_importance)
    {
        for (int vi = 0; vi < var_importance->cols; vi++)
                var_importance->tt.fl[vi] = (var_importance->tt.fl[vi] > 0) ?
                    var_importance->tt.fl[vi] : 0;
        cvNormalize(var_importance, var_importance, 1., 0, CC_L1);
    }

    cFree(&oob_samples_perm_ptr);
    cFree(&samples_ptr);
    cFree(&missing_ptr);
    cFree(&true_resp_ptr);

    cvReleaseMat(&sample_idx_mask_for_tree);
    cvReleaseMat(&sample_idx_for_tree);

    cvReleaseMat(&oob_sample_votes);
    cvReleaseMat(&oob_responses);

    return true;
}


const img_t* CvRTrees::get_var_importance()
{
    return var_importance;
}


float CvRTrees::get_proximity(const img_t* sample1, const img_t* sample2,
                              const img_t* missing1, const img_t* missing2) const
{
    float result = 0;

    for(int i = 0; i < ntrees; i++)
        result += trees[i]->predict(sample1, missing1) ==
        trees[i]->predict(sample2, missing2) ?  1 : 0;
    result = result/(float)ntrees;

    return result;
}

float CvRTrees::calc_error(CvMLData* _data, int type , std::vector<float> *resp)
{
    float err = 0;
    const img_t* values = _data->get_values();
    const img_t* response = _data->get_responses();
    const img_t* missing = _data->get_missing();
    const img_t* sample_idx = (type == CC_TEST_ERROR) ? _data->get_test_sample_idx() : _data->get_train_sample_idx();
    const img_t* var_types = _data->get_var_types();
    int* sidx = sample_idx ? sample_idx->tt.i : 0;
    int r_step = CC_IS_MAT_CONT(response->tid) ?
                1 : response->step / CC_ELEM_SIZE(response->tid);
    bool is_classifier = var_types->tt.data[var_types->cols-1] == CC_VAR_CATEGORICAL;
    int sample_count = sample_idx ? sample_idx->cols : 0;
    sample_count = (type == CC_TRAIN_ERROR && sample_count == 0) ? values->rows : sample_count;
    float* pred_resp = 0;
    if(resp && (sample_count > 0))
    {
        resp->resize(sample_count);
        pred_resp = &((*resp)[0]);
    }
    if (is_classifier)
    {
        for(int i = 0; i < sample_count; i++)
        {
            img_t sample, miss;
            int si = sidx ? sidx[i] : i;
            cvGetRow(values, &sample, si);
            if(missing)
                cvGetRow(missing, &miss, si);
            float r = (float)predict(&sample, missing ? &miss : 0);
            if(pred_resp)
                pred_resp[i] = r;
            int d = fabs((double)r - response->tt.fl[si*r_step]) <= FLT_EPSILON ? 0 : 1;
            err += d;
        }
        err = sample_count ? err / (float)sample_count * 100 : -FLT_MAX;
    }
    else
    {
        for(int i = 0; i < sample_count; i++)
        {
            img_t sample, miss;
            int si = sidx ? sidx[i] : i;
            cvGetRow(values, &sample, si);
            if(missing)
                cvGetRow(missing, &miss, si);
            float r = (float)predict(&sample, missing ? &miss : 0);
            if(pred_resp)
                pred_resp[i] = r;
            float d = r - response->tt.fl[si*r_step];
            err += d*d;
        }
        err = sample_count ? err / (float)sample_count : -FLT_MAX;
    }
    return err;
}

float CvRTrees::get_train_error()
{
    float err = -1;

    int sample_count = data->sample_count;
    int var_count = data->var_count;

    float *values_ptr = (float*)cAlloc(sizeof(float)*sample_count*var_count);
    uchar *missing_ptr = (uchar*)cAlloc(sizeof(uchar)*sample_count*var_count);
    float *responses_ptr = (float*)cAlloc(sizeof(float)*sample_count);

    data->get_vectors(0, values_ptr, missing_ptr, responses_ptr);

    if (data->is_classifier)
    {
        int err_count = 0;
        float *vp = values_ptr;
        uchar *mp = missing_ptr;
        for (int si = 0; si < sample_count; si++, vp += var_count, mp += var_count)
        {
            img_t sample = cvMat(1, var_count, CC_32FC1, vp);
            img_t missing = cvMat(1, var_count, CC_8UC1,  mp);
            float r = predict(&sample, &missing);
            if (fabs(r - responses_ptr[si]) >= FLT_EPSILON)
                err_count++;
        }
        err = (float)err_count / (float)sample_count;
    }
    else
        CC_Error(CC_StsBadArg, "This method is not supported for regression problems");

    cFree(&values_ptr);
    cFree(&missing_ptr);
    cFree(&responses_ptr);

    return err;
}


float CvRTrees::predict(const img_t* sample, const img_t* missing) const
{
    double result = -1;
    int k;

    if(nclasses > 0) //classification
    {
        int max_nvotes = 0;
        FREE(); int* = MALLOC(int, ) _votes(nclasses);
        int* votes = _votes;
        memset(votes, 0, sizeof(*votes)*nclasses);
        for(k = 0; k < ntrees; k++)
        {
            CvDTreeNode* predicted_node = trees[k]->predict(sample, missing);
            int nvotes;
            int class_idx = predicted_node->class_idx;
            CC_Assert(0 <= class_idx && class_idx < nclasses);

            nvotes = ++votes[class_idx];
            if(nvotes > max_nvotes)
            {
                max_nvotes = nvotes;
                result = predicted_node->value;
            }
        }
    }
    else // regression
    {
        result = 0;
        for(k = 0; k < ntrees; k++)
            result += trees[k]->predict(sample, missing)->value;
        result /= (double)ntrees;
    }

    return (float)result;
}

float CvRTrees::predict_prob(const img_t* sample, const img_t* missing) const
{
    if(nclasses == 2) //classification
    {
        FREE(); int* = MALLOC(int, ) _votes(nclasses);
        int* votes = _votes;
        memset(votes, 0, sizeof(*votes)*nclasses);
        for(int k = 0; k < ntrees; k++)
        {
            CvDTreeNode* predicted_node = trees[k]->predict(sample, missing);
            int class_idx = predicted_node->class_idx;
            CC_Assert(0 <= class_idx && class_idx < nclasses);

            ++votes[class_idx];
        }

        return float(votes[1])/ntrees;
    }
    else // regression
        CC_Error(CC_StsBadArg, "This function works for binary classification problems only...");

    return -1;
}

void CvRTrees::write(CvFileStorage* fs, const char* name) const
{
    int k;

    if(ntrees < 1 || !trees || nsamples < 1)
        CC_Error(CC_StsBadArg, "Invalid CvRTrees object");

    std::string modelNodeName = this->getName();
    cvWriteStructBegin(fs, name, CC_NODE_MAP, modelNodeName.c_str());

    cvWriteInt(fs, "nclasses", nclasses);
    cvWriteInt(fs, "nsamples", nsamples);
    cvWriteInt(fs, "nactive_vars", (int)cvSum(active_var_mask).val[0]);
    cvWriteReal(fs, "oob_error", oob_error);

    if(var_importance)
        cvWrite(fs, "var_importance", var_importance);

    cvWriteInt(fs, "ntrees", ntrees);

    data->write_params(fs);

    cvWriteStructBegin(fs, "trees", CC_NODE_SEQ);

    for(k = 0; k < ntrees; k++)
    {
        cvWriteStructBegin(fs, 0, CC_NODE_MAP);
        trees[k]->write(fs);
        cvWriteStructEnd(fs);
    }

    cvWriteStructEnd(fs); //trees
    cvWriteStructEnd(fs); //CC_TYPE_NAME_ML_RTREES
}


void CvRTrees::read(CvFileStorage* fs, CvFileNode* fnode)
{
    int nactive_vars, var_count, k;
    CvSeqReader reader;
    CvFileNode* trees_fnode = 0;

    clear();

    nclasses     = cvReadIntByName(fs, fnode, "nclasses", -1);
    nsamples     = cvReadIntByName(fs, fnode, "nsamples");
    nactive_vars = cvReadIntByName(fs, fnode, "nactive_vars", -1);
    oob_error    = cvReadRealByName(fs, fnode, "oob_error", -1);
    ntrees       = cvReadIntByName(fs, fnode, "ntrees", -1);

    var_importance = cvReadByName(fs, fnode, "var_importance");

    if(nclasses < 0 || nsamples <= 0 || nactive_vars < 0 || oob_error < 0 || ntrees <= 0)
        CC_Error(CC_StsParseError, "Some <nclasses>, <nsamples>, <var_count>, "
        "<nactive_vars>, <oob_error>, <ntrees> of tags are missing");

    rng = &theRNG();

    trees = (CvForestTree**)cAlloc(sizeof(trees[0])*ntrees);
    memset(trees, 0, sizeof(trees[0])*ntrees);

    data = new CvDTreeTrainData();
    data->read_params(fs, fnode);
    data->shared = true;

    trees_fnode = cvGetFileNodeByName(fs, fnode, "trees");
    if(!trees_fnode || !CC_NODE_IS_SEQ(trees_fnode->tag))
        CC_Error(CC_StsParseError, "<trees> tag is missing");

    cvStartReadSeq(trees_fnode->tt.seq, &reader);
    if(reader.seq->total != ntrees)
        CC_Error(CC_StsParseError,
        "<ntrees> is not equal to the number of trees saved in file");

    for(k = 0; k < ntrees; k++)
    {
        trees[k] = new CvForestTree();
        trees[k]->read(fs, (CvFileNode*)reader.ptr, this, data);
        CC_NEXT_SEQ_ELEM(reader.seq->elem_size, reader);
    }

    var_count = data->var_count;
    active_var_mask = cvCreateMat(1, var_count, CC_8UC1);
    {
        // initialize active variables mask
        img_t submask1;
        cvGetCols(active_var_mask, &submask1, 0, nactive_vars);
        cvSet(&submask1, cScalar(1));

        if(nactive_vars < var_count)
        {
            img_t submask2;
            cvGetCols(active_var_mask, &submask2, nactive_vars, var_count);
            cvZero(&submask2);
        }
    }
}


int CvRTrees::get_tree_count() const
{
    return ntrees;
}

CvForestTree* CvRTrees::get_tree(int i) const
{
    return (unsigned)i < (unsigned)ntrees ? trees[i] : 0;
}

using namespace cv;

bool CvRTrees::train(const img_t& _train_data, int _tflag,
                     const img_t& _responses, const img_t& _var_idx,
                     const img_t& _sample_idx, const img_t& _var_type,
                     const img_t& _missing_mask, CvRTParams _params)
{
    img_t tdata = _train_data, responses = _responses, vidx = _var_idx,
    sidx = _sample_idx, vtype = _var_type, mmask = _missing_mask;
    return train(&tdata, _tflag, &responses, vidx->tt.data ? &vidx : 0,
                 sidx->tt.data ? &sidx : 0, vtype->tt.data ? &vtype : 0,
                 mmask->tt.data ? &mmask : 0, _params);
}


float CvRTrees::predict(const img_t& _sample, const img_t& _missing) const
{
    img_t sample = _sample, mmask = _missing;
    return predict(&sample, mmask->tt.data ? &mmask : 0);
}

float CvRTrees::predict_prob(const img_t& _sample, const img_t& _missing) const
{
    img_t sample = _sample, mmask = _missing;
    return predict_prob(&sample, mmask->tt.data ? &mmask : 0);
}

img_t CvRTrees::getVarImportance()
{
    return img_t(get_var_importance());
}

// End of file.