#include "funset.hpp"
#include <iostream>
#include "perceptron.hpp"
#include "BP.hpp""
#include "CNN.hpp"
#include "linear_regression.hpp"
#include "naive_bayes_classifier.hpp"
#include "logistic_regression.hpp"
#include "common.hpp"
#include "knn.hpp"
#include "decision_tree.hpp"
#include "pca.hpp"
#include <opencv2/opencv.hpp>

// =============================== PCA(Principal Components Analysis) ===================
namespace {
void normalize(const std::vector<float>& src, std::vector<unsigned char>& dst)
{
	dst.resize(src.size());
	double dmin = 0, dmax = 255;
	double smin = src[0], smax = smin;

	for (int i = 1; i < src.size(); ++i) {
		if (smin > src[i]) smin = src[i];
		if (smax < src[i]) smax = src[i];
	}

	double scale = (dmax - dmin) * (smax - smin > DBL_EPSILON ? 1. / (smax - smin) : 0);
	double shift = dmin - smin * scale;

	for (int i = 0; i < src.size(); ++i) {
		dst[i] = static_cast<unsigned char>(src[i] * scale + shift);
	}
}
} // namespace

int test_pca()
{
	const std::string image_path{ "E:/GitCode/NN_Test/data/database/ORL_Faces/" };
	const std::string image_name{ "1.pgm" };

	std::vector<cv::Mat> images;
	for (int i = 1; i <= 15; ++i) {
		std::string name = image_path + "s" + std::to_string(i) + "/" + image_name;
		cv::Mat mat = cv::imread(name, 0);
		if (!mat.data) {
			fprintf(stderr, "read image fail: %s\n", name.c_str());
			return -1;
		}

		images.emplace_back(mat);
	}
	save_images(images, "E:/GitCode/NN_Test/data/pca_src.jpg", 5);

	cv::Mat data(images.size(), images[0].rows * images[0].cols, CV_32FC1);
	for (int i = 0; i < images.size(); ++i) {
		cv::Mat image_row = images[i].clone().reshape(1, 1);
		cv::Mat row_i = data.row(i);
		image_row.convertTo(row_i, CV_32F);
	}

	int features_length = images[0].rows * images[0].cols;
	std::vector<std::vector<float>> data_(images.size());
	std::vector<float> labels(images.size(), 0.f);
	for (int i = 0; i < images.size(); ++i) {
		data_[i].resize(features_length);
		memcpy(data_[i].data(), data.row(i).data, sizeof(float)* features_length);
	}
	
	const std::string save_model_file{ "E:/GitCode/NN_Test/data/pca.model" };
	ANN::PCA<float> pca;
	pca.load_data(data_, labels);
	double retained_variance{ 0.95 };
	pca.set_retained_variance(retained_variance);
	pca.train(save_model_file);

	const std::string read_model_file{ save_model_file };
	ANN::PCA<float> pca2;
	pca2.load_model(read_model_file);

	std::vector<cv::Mat> result(images.size());
	for (int i = 0; i < images.size(); ++i) {
		std::vector<float> point, reconstruction;
		pca2.project(data_[i], point);
		pca2.back_project(point, reconstruction);
		std::vector<unsigned char> dst;
		normalize(reconstruction, dst);
		cv::Mat tmp(images[i].rows, images[i].cols, CV_8UC1, dst.data());
		tmp.copyTo(result[i]);
	}
	save_images(result, "E:/GitCode/NN_Test/data/pca_result.jpg", 5);

	return 0;
}

// =============================== decision tree ==============================
int test_decision_tree_train()
{
	/*const std::vector<std::vector<float>> data{ { 1.f, 3.f, 4.f, 5.f, 1.f }, { 1.f, 8.f, 5.f, 5.f, 0.f },
	{ 1.f, 2.f, 7.f, 3.f, 1.f }, { 1.f, 9.f, 3.f, 3.f, 0.f },
	{ -2.f, 4.f, 0.f, 3.f, 0.f }, {9.f, 3.f, 4.f, -1.f, 0.f} };*/
	const std::vector<std::vector<float>> data{ { 2.771244718f, 1.784783929f, 0.f },
					{ 1.728571309f, 1.169761413f, 0.f },
					{ 3.678319846f, 2.81281357f, 0.f },
					{ 3.961043357f, 2.61995032f, 0.f },
					{ 2.999208922f, 2.209014212f, 0.f },
					{ 7.497545867f, 3.162953546f, 1.f },
					{ 9.00220326f, 3.339047188f, 1.f },
					{ 7.444542326f, 0.476683375f, 1.f },
					{ 10.12493903f, 3.234550982f, 1.f },
					{ 6.642287351f, 3.319983761f, 1.f } };

	const std::vector<float> classes{ 0.f, 1.f };

	ANN::DecisionTree<float> dt;
	dt.init(data, classes);

	return 0;
}

int test_decision_tree_predict()
{
	return 0;
}

// =========================== KNN(K-Nearest Neighbor) ======================
int test_knn_classifier_predict()
{
	const std::string image_path{ "E:/GitCode/NN_Test/data/images/digit/handwriting_0_and_1/" };
	const int K{ 3 };

	cv::Mat tmp = cv::imread(image_path + "0_1.jpg", 0);
	const int train_samples_number{ 40 }, predict_samples_number{ 40 };
	const int every_class_number{ 10 };

	cv::Mat train_data(train_samples_number, tmp.rows * tmp.cols, CV_32FC1);
	cv::Mat train_labels(train_samples_number, 1, CV_32FC1);
	float* p = (float*)train_labels.data;
	for (int i = 0; i < 4; ++i) {
		std::for_each(p + i * every_class_number, p + (i + 1)*every_class_number, [i](float& v){v = (float)i; });
	}

	// train data
	for (int i = 0; i < 4; ++i) {
		static const std::vector<std::string> digit{ "0_", "1_", "2_", "3_" };
		static const std::string suffix{ ".jpg" };

		for (int j = 1; j <= every_class_number; ++j) {
			std::string image_name = image_path + digit[i] + std::to_string(j) + suffix;
			cv::Mat image = cv::imread(image_name, 0);
			CHECK(!image.empty() && image.isContinuous());
			image.convertTo(image, CV_32FC1);

			image = image.reshape(0, 1);
			tmp = train_data.rowRange(i * every_class_number + j - 1, i * every_class_number + j);
			image.copyTo(tmp);
		}
	}

	ANN::KNN<float> knn;
	knn.set_k(K);

	std::vector<std::vector<float>> samples(train_samples_number);
	std::vector<float> labels(train_samples_number);
	const int feature_length{ tmp.rows * tmp.cols };

	for (int i = 0; i < train_samples_number; ++i) {
		samples[i].resize(feature_length);
		const float* p1 = train_data.ptr<float>(i);
		float* p2 = samples[i].data();

		memcpy(p2, p1, feature_length * sizeof(float));
	}

	const float* p1 = (const float*)train_labels.data;
	float* p2 = labels.data();
	memcpy(p2, p1, train_samples_number * sizeof(float));

	knn.set_train_samples(samples, labels);

	// predict datta
	cv::Mat predict_data(predict_samples_number, tmp.rows * tmp.cols, CV_32FC1);
	for (int i = 0; i < 4; ++i) {
		static const std::vector<std::string> digit{ "0_", "1_", "2_", "3_" };
		static const std::string suffix{ ".jpg" };

		for (int j = 11; j <= every_class_number + 10; ++j) {
			std::string image_name = image_path + digit[i] + std::to_string(j) + suffix;
			cv::Mat image = cv::imread(image_name, 0);
			CHECK(!image.empty() && image.isContinuous());
			image.convertTo(image, CV_32FC1);

			image = image.reshape(0, 1);
			tmp = predict_data.rowRange(i * every_class_number + j - 10 - 1, i * every_class_number + j - 10);
			image.copyTo(tmp);
		}
	}

	cv::Mat predict_labels(predict_samples_number, 1, CV_32FC1);
	p = (float*)predict_labels.data;
	for (int i = 0; i < 4; ++i) {
		std::for_each(p + i * every_class_number, p + (i + 1)*every_class_number, [i](float& v){v = (float)i; });
	}

	std::vector<float> sample(feature_length);
	int count{ 0 };
	for (int i = 0; i < predict_samples_number; ++i) {
		float value1 = ((float*)predict_labels.data)[i];
		float value2;
		memcpy(sample.data(), predict_data.ptr<float>(i), feature_length * sizeof(float));

		CHECK(knn.predict(sample, value2) == 0);
		fprintf(stdout, "expected value: %f, actual value: %f\n", value1, value2);

		if (int(value1) == int(value2)) ++count;
	}
	fprintf(stdout, "when K = %d, accuracy: %f\n", K, count * 1.f / predict_samples_number);

	return 0;
}

// ================================ logistic regression =====================
int test_logistic_regression_train()
{
	const std::string image_path{ "E:/GitCode/NN_Test/data/images/digit/handwriting_0_and_1/" };
	cv::Mat data, labels;

	for (int i = 1; i < 11; ++i) {
		const std::vector<std::string> label{ "0_", "1_" };

		for (const auto& value : label) {
			std::string name = std::to_string(i);
			name = image_path + value + name + ".jpg";

			cv::Mat image = cv::imread(name, 0);
			if (image.empty()) {
				fprintf(stderr, "read image fail: %s\n", name.c_str());
				return -1;
			}

			data.push_back(image.reshape(0, 1));
		}
	}
	data.convertTo(data, CV_32F);

	std::unique_ptr<float[]> tmp(new float[20]);
	for (int i = 0; i < 20; ++i) {
		if (i % 2 == 0) tmp[i] = 0.f;
		else tmp[i] = 1.f;
	}
	labels = cv::Mat(20, 1, CV_32FC1, tmp.get());

	ANN::LogisticRegression<float> lr;
	const float learning_rate{ 0.00001f };
	const int iterations{ 250 };
	int reg_kinds = lr.REG_DISABLE; //ANN::LogisticRegression<float>::REG_L1;
	int train_method = lr.MINI_BATCH; //ANN::LogisticRegression<float>::BATCH;
	int mini_batch_size = 5;

	int ret = lr.init((float*)data.data, (float*)labels.data, data.rows, data.cols/*,
		reg_kinds, learning_rate, iterations, train_method, mini_batch_size*/);
	if (ret != 0) {
		fprintf(stderr, "logistic regression init fail: %d\n", ret);
		return -1;
	}

	const std::string model{ "E:/GitCode/NN_Test/data/logistic_regression.model" };

	ret = lr.train(model);
	if (ret != 0) {
		fprintf(stderr, "logistic regression train fail: %d\n", ret);
		return -1;
	}

	return 0;
}

int test_logistic_regression_predict()
{
	const std::string image_path{ "E:/GitCode/NN_Test/data/images/digit/handwriting_0_and_1/" };
	cv::Mat data, labels, result;

	for (int i = 11; i < 21; ++i) {
		const std::vector<std::string> label{ "0_", "1_" };

		for (const auto& value : label) {
			std::string name = std::to_string(i);
			name = image_path + value + name + ".jpg";

			cv::Mat image = cv::imread(name, 0);
			if (image.empty()) {
				fprintf(stderr, "read image fail: %s\n", name.c_str());
				return -1;
			}

			data.push_back(image.reshape(0, 1));
		}
	}
	data.convertTo(data, CV_32F);

	std::unique_ptr<int[]> tmp(new int[20]);
	for (int i = 0; i < 20; ++i) {
		if (i % 2 == 0) tmp[i] = 0;
		else tmp[i] = 1;
	}
	labels = cv::Mat(20, 1, CV_32SC1, tmp.get());

	CHECK(data.rows == labels.rows);

	const std::string model{ "E:/GitCode/NN_Test/data/logistic_regression.model" };

	ANN::LogisticRegression<float> lr;
	int ret = lr.load_model(model);
	if (ret != 0) {
		fprintf(stderr, "load logistic regression model fail: %d\n", ret);
		return -1;
	}

	for (int i = 0; i < data.rows; ++i) {
		float probability = lr.predict((float*)(data.row(i).data), data.cols);

		fprintf(stdout, "probability: %.6f, ", probability);
		if (probability > 0.5) fprintf(stdout, "predict result: 1, ");
		else fprintf(stdout, "predict result: 0, ");
		fprintf(stdout, "actual result: %d\n", ((int*)(labels.row(i).data))[0]);
	}

	return 0;
}

// ================================ naive bayes classifier =====================
int test_naive_bayes_classifier_train()
{
	std::vector<ANN::sex_info<float>> info;
	info.push_back({ 6.f, 180.f, 12.f, 1 });
	info.push_back({5.92f, 190.f, 11.f, 1});
	info.push_back({5.58f, 170.f, 12.f, 1});
	info.push_back({5.92f, 165.f, 10.f, 1});
	info.push_back({ 5.f, 100.f, 6.f, 0 });
	info.push_back({5.5f, 150.f, 8.f, 0});
	info.push_back({5.42f, 130.f, 7.f, 0});
	info.push_back({5.75f, 150.f, 9.f, 0});

	ANN::NaiveBayesClassifier<float> naive_bayes;
	int ret = naive_bayes.init(info);
	if (ret != 0) {
		fprintf(stderr, "naive bayes classifier init fail: %d\n", ret);
		return -1;
	}

	const std::string model{ "E:/GitCode/NN_Test/data/naive_bayes_classifier.model" };
	ret = naive_bayes.train(model);
	if (ret != 0) {
		fprintf(stderr, "naive bayes classifier train fail: %d\n", ret);
		return -1;
	}

	return 0;
}

int test_naive_bayes_classifier_predict()
{
	ANN::sex_info<float> info = { 6.0f, 130.f, 8.f, -1 };

	ANN::NaiveBayesClassifier<float> naive_bayes;
	const std::string model{ "E:/GitCode/NN_Test/data/naive_bayes_classifier.model" };
	int ret = naive_bayes.load_model(model);
	if (ret != 0) {
		fprintf(stderr, "load naive bayes classifier model fail: %d\n", ret);
		return -1;
	}

	ret = naive_bayes.predict(info);
	if (ret == 0) fprintf(stdout, "It is a female\n");
	else fprintf(stdout, "It is a male\n");

	return 0;
}

// ============================ linear regression =============================
int test_linear_regression_train()
{
	std::vector<float> x{6.2f, 9.5f, 10.5f, 7.7f, 8.6f, 6.9f, 7.3f, 2.2f, 5.7f, 2.f,
		2.5f, 4.f, 5.4f, 2.2f, 7.2f, 12.2f, 5.6f, 9.f, 3.6f, 5.f,
		11.3f, 3.4f, 11.9f, 10.5f, 10.7f, 10.8f, 4.8f};
	std::vector<float> y{ 29.f, 44.f, 36.f, 37.f, 53.f, 18.f, 31.f, 14.f, 11.f, 11.f,
		22.f, 16.f, 27.f, 9.f, 29.f, 46.f, 23.f, 39.f, 15.f, 32.f,
		34.f, 17.f, 46.f, 42.f, 43.f, 34.f, 19.f };
	CHECK(x.size() == y.size());

	ANN::LinearRegression<float> lr;

	lr.set_regression_method(ANN::GRADIENT_DESCENT);
	lr.init(x.data(), y.data(), x.size());

	float learning_rate{ 0.001f };
	int iterations{ 1000 };
	std::string model{ "E:/GitCode/NN_Test/data/linear_regression.model" };

	int ret = lr.train(model, learning_rate, iterations);
	if (ret != 0) {
		fprintf(stderr, "train fail\n");
		return -1;
	}

	std::cout << lr << std::endl; // y = wx + b

	return 0;
}
int test_linear_regression_predict()
{
	ANN::LinearRegression<float> lr;

	std::string model{ "E:/GitCode/NN_Test/data/linear_regression.model" };
	int ret = lr.load_model(model);
	if (ret != 0) {
		fprintf(stderr, "load model fail: %s\n", model.c_str());
		return -1;
	}

	float x = 13.8f;
	float result = lr.predict(x);
	fprintf(stdout, "input value: %f, result value: %f\n", x, result);

	return 0;
}

// =============================== perceptron =================================
int test_perceptron()
{
	// prepare data
	const int len_data = 20;
	const int feature_dimension = 2;
	float data[len_data][feature_dimension] = {
		{ 10.3, 10.7 }, { 20.1, 100.8 }, { 44.9, 8.0 }, { -2.2, 15.3 }, { -33.3, 77.7 },
		{ -10.4, 111.1 }, { 99.3, -2.2 }, { 222.2, -5.5 }, { 10.1, 10.1 }, { 66.6, 30.2 },
		{ 0.1, 0.2 }, { 1.2, 0.03 }, { 0.5, 4.6 }, { -22.3, -11.1 }, { -88.9, -12.3 },
		{ -333.3, -444.4 }, { -111.2, 0.5 }, { -6.6, 2.9 }, { 3.3, -100.2 }, { 5.6, -88.8 } };
	int label_[len_data] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	std::vector<ANN::feature> set_feature;
	std::vector<ANN::label> set_label;

	for (int i = 0; i < len_data; i++) {
		ANN::feature feature_single;
		for (int j = 0; j < feature_dimension; j++) {
			feature_single.push_back(data[i][j]);
		}

		set_feature.push_back(feature_single);
		set_label.push_back(label_[i]);

		feature_single.resize(0);
	}

	// train
	int iterates = 1000;
	float learn_rate = 0.5;
	int size_weight = feature_dimension;
	float bias = 2.5;
	ANN::Perceptron perceptron(iterates, learn_rate, size_weight, bias);
	perceptron.getDataset(set_feature, set_label);
	bool flag = perceptron.train();
	if (flag) {
		std::cout << "data set is linearly separable" << std::endl;
	}
	else {
		std::cout << "data set is linearly inseparable" << std::endl;
		return -1;
	}

	// predict
	ANN::feature feature1;
	feature1.push_back(636.6);
	feature1.push_back(881.8);
	std::cout << "the correct result label is 1, " << "the real result label is: " << perceptron.predict(feature1) << std::endl;

	ANN::feature feature2;
	feature2.push_back(-26.32);
	feature2.push_back(-255.95);
	std::cout << "the correct result label is -1, " << "the real result label is: " << perceptron.predict(feature2) << std::endl;

	return 0;
}

// =================================== BP =====================================
int test_BP_train()
{
	ANN::BP bp1;
	bp1.init();
	bp1.train();

	return 0;
}

int test_BP_predict()
{
	ANN::BP bp2;
	bool flag = bp2.readModelFile("E:/GitCode/NN_Test/data/bp.model");
	if (!flag) {
		std::cout << "read bp model error" << std::endl;
		return -1;
	}

	int target[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::string path_images = "E:/GitCode/NN_Test/data/images/digit/handwriting_1/";

	int* data_image = new int[width_image_BP * height_image_BP];

	for (int i = 0; i < 10; i++) {
		char ch[15];
		sprintf(ch, "%d", i);
		std::string str;
		str = std::string(ch);
		str += ".jpg";
		str = path_images + str;

		cv::Mat mat = cv::imread(str, 2 | 4);
		if (!mat.data) {
			std::cout << "read image error" << std::endl;
			return -1;
		}

		if (mat.channels() == 3) {
			cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
		}

		if (mat.cols != width_image_BP || mat.rows != height_image_BP) {
			cv::resize(mat, mat, cv::Size(width_image_BP, height_image_BP));
		}

		memset(data_image, 0, sizeof(int) * (width_image_BP * height_image_BP));

		for (int h = 0; h < mat.rows; h++) {
			uchar* p = mat.ptr(h);
			for (int w = 0; w < mat.cols; w++) {
				if (p[w] > 128) {
					data_image[h* mat.cols + w] = 1;
				}
			}
		}

		int ret = bp2.predict(data_image, mat.cols, mat.rows);
		std::cout << "correct result: " << i << ",    actual result: " << ret << std::endl;
	}

	delete[] data_image;

	return 0;
}

// =================================== CNN ====================================
int test_CNN_train()
{
	ANN::CNN cnn1;
	cnn1.init();
	cnn1.train();

	return 0;
}

int test_CNN_predict()
{
	ANN::CNN cnn2;
	bool flag = cnn2.readModelFile("E:/GitCode/NN_Test/data/cnn.model");
	if (!flag) {
		std::cout << "read cnn model error" << std::endl;
		return -1;
	}

	int width{ 32 }, height{ 32 };
	std::vector<int> target{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::string image_path{ "E:/GitCode/NN_Test/data/images/digit/handwriting_2/" };

	for (auto i : target) {
		std::string str = std::to_string(i);
		str += ".png";
		str = image_path + str;

		cv::Mat src = cv::imread(str, 0);
		if (src.data == nullptr) {
			fprintf(stderr, "read image error: %s\n", str.c_str());
			return -1;
		}

		cv::Mat tmp(src.rows, src.cols, CV_8UC1, cv::Scalar::all(255));
		cv::subtract(tmp, src, tmp);

		cv::resize(tmp, tmp, cv::Size(width, height));

		auto ret = cnn2.predict(tmp.data, width, height);

		fprintf(stdout, "the actual digit is: %d, correct digit is: %d\n", ret, i);
	}

	return 0;
}

