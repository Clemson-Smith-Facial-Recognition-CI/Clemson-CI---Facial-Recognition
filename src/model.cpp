/**
 * @file model.cpp
 *
 * Implementation of the model type.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "logger.h"
#include "model.h"
#include "timer.h"

/**
 * Construct a model.
 *
 * @param feature
 * @param classifier
 */
Model::Model(FeatureLayer *feature, ClassifierLayer *classifier)
{
	// initialize layers
	this->feature = feature;
	this->classifier = classifier;

	// initialize stats
	this->stats.accuracy = 0.0f;
	this->stats.train_time = 0.0f;
	this->stats.test_time = 0.0f;

	// log hyperparameters
	log(LL_VERBOSE, "Hyperparameters\n");

	this->feature->print();
	this->classifier->print();

	log(LL_VERBOSE, "\n");
}

/**
 * Destruct a model.
 */
Model::~Model()
{
	delete this->feature;
	delete this->classifier;
}

/**
 * Perform training on a training set.
 *
 * @param train_set
 */
void Model::train(const Dataset& train_set)
{
	timer_push("Training");

	this->train_set = train_set;

	log(LL_VERBOSE, "Training set: %d samples, %d classes\n",
		train_set.entries.size(),
		train_set.labels.size());

	// get data matrix X
	Matrix X = train_set.load();

	// subtract mean from X
	this->mean = X.mean_column("m");

	X.subtract_columns(this->mean);

	// project X into feature space
	this->feature->compute(X, this->train_set.entries, this->train_set.labels.size());
	this->P = this->feature->project(X);

	// record training time
	this->stats.train_time = timer_pop();

	log(LL_VERBOSE, "\n");
}

/**
 * Save a model to a file.
 *
 * @param path
 */
void Model::save(const char *path)
{
	FILE *file = fopen(path, "w");

	this->train_set.save(file);

	this->mean.save(file);

	this->feature->save(file);
	this->P.save(file);

	fclose(file);
}

/**
 * Load a model from a file.
 *
 * @param path
 */
void Model::load(const char *path)
{
	FILE *file = fopen(path, "r");

	this->train_set = Dataset(file);

	this->mean.load(file);

	this->feature->load(file);
	this->P.load(file);

	fclose(file);
}

/**
 * Perform recognition on a test set.
 *
 * @param test_set
 */
std::vector<data_label_t> Model::predict(const Dataset& test_set)
{
	timer_push("Prediction");

	log(LL_VERBOSE, "Test set: %d samples, %d classes\n",
		test_set.entries.size(),
		test_set.labels.size());

	// compute projected test images
	Matrix X_test = test_set.load();
	X_test.subtract_columns(this->mean);

	Matrix P_test = this->feature->project(X_test);

	// compute predicted labels
	std::vector<data_label_t> Y_pred = this->classifier->predict(
		this->P,
		this->train_set.entries,
		this->train_set.labels,
		P_test);

	// record predition time
	this->stats.test_time = timer_pop();

	log(LL_VERBOSE, "\n");

	return Y_pred;
}

/**
 * Validate a set of predicted labels against the ground truth.
 *
 * @param test_set
 * @param Y_pred
 */
void Model::validate(const Dataset& test_set, const std::vector<data_label_t>& Y_pred)
{
	// compute accuracy
	int num_correct = 0;

	unsigned i;
	for ( i = 0; i < test_set.entries.size(); i++ ) {
		if ( Y_pred[i] == test_set.entries[i].label ) {
			num_correct++;
		}
	}

	this->stats.accuracy = 100.0f * num_correct / test_set.entries.size();

	// print results
	log(LL_VERBOSE, "Results\n");

	for ( i = 0; i < test_set.entries.size(); i++ ) {
		const data_label_t& y_pred = Y_pred[i];
		const data_entry_t& entry = test_set.entries[i];

		const char *s = (y_pred != entry.label)
			? "(!)"
			: "";

		log(LL_VERBOSE, "%-12s -> %-4s %s\n",
			basename(entry.name.c_str()),
			y_pred.c_str(), s);
	}

	log(LL_VERBOSE, "%d / %d matched, %.2f%%\n",
		num_correct,
		test_set.entries.size(),
		this->stats.accuracy);
	log(LL_VERBOSE, "\n");
}

/**
 * Print a model's performance / accuracy stats.
 */
void Model::print_stats()
{
	printf("%10.2f  %10.3f  %10.3f\n",
		this->stats.accuracy,
		this->stats.train_time,
		this->stats.test_time);
}
