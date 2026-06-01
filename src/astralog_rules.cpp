#include"astralog_rules.hpp"

namespace astralog::rules {
    std::shared_ptr<Rule> Rule::from_json(const json &json, std::vector<std::shared_ptr<Rule>> &rules) {

        // check mandatory fields
        //check id
  if (!json.contains("id") ) {
    return nullptr;
  }
  std::string id = json["id"].get<std::string>();
  if (id[0]!='R') {
    return nullptr;
  }
  //check sensor_id
  if (!json.contains("sensor_id")) {
    return nullptr;
  }
  std::string sensor_id = json["sensor_id"].get<std::string>();

    //check threshold
    if (!json.contains("value") || !json["value"].is_number()) {
    return nullptr;
    }
    double threshold = json["value"].get<double>();


    //check optional field
    //check priority
    if (!json.contains("priority")) {
    return nullptr;
    }
  std::string priority_str = json["priority"].get<std::string>();
  std::optional<Priority> priority = std::nullopt;
  if(priority_str == "LOW") {
    priority = Priority::LOW;
  } else if(priority_str == "MEDIUM") {
    priority = Priority::MEDIUM;
  } else if(priority_str == "HIGH") {
    priority = Priority::HIGH;
  } else {
    return nullptr;
  }

  //check type
  if (!json.contains("type") ) {
    return nullptr;
  }
  std::string type_str = json["type"].get<std::string>();
  if(type_str!="SIMPLE" && type_str!="STEP_DIFFERENCE" && type_str!="STATEFUL" && type_str!="CORRELATION"){
    return nullptr;
  }
  RuleType type;
    switch (type_str[2]) {
    case 'M':
       type = RuleType::SIMPLE;
        break;
    case 'E':
        type = RuleType::STEP_DIFFERENCE;
        break;
    case 'A':
        type = RuleType::STATEFUL;
        break;
    case 'R':
        type = RuleType::CORRELATION;
        break;
    default:
        return nullptr;
    }

    //check operator and logic
    if(type == RuleType::CORRELATION)
    {
        if (!json.contains("logic")) {
            return nullptr;
        }
        std::string logic = json["logic"].get<std::string>();
        if(logic != "AND" && logic != "OR") {
            return nullptr;
        }
        if(!json.contains("conditions"))
        {
            return nullptr;
        }
        if(!json["conditions"].is_array()&&(json["conditions"].size()!=2)) {
            return nullptr;
        }
        std::vector<std::string> conditions = json["conditions"].get<std::vector<std::string>>();
        for(const auto &condition : conditions) {
            if(condition[0]!='R') {
                return nullptr;
            }
        }
        std::vector<std::shared_ptr<Rule>> parent_rules;
        for(int i=0;i<conditions.size();++i) {
            for(int j=0;j<rules.size();++j) {
                if(rules[j]->id() == conditions[i]) {
                    parent_rules.push_back(std::make_shared<Rule>(rules[j]));
                }
            }
        }
        CorrelationRule rule = CorrelationRule(id, priority, logic, parent_rules);
        return std::make_shared<CorrelationRule>(rule);
    } else {
        if (!json.contains("operator")) {
            return nullptr;
        }
        std::string op_str = json["operator"].get<std::string>();
        if(op_str != ">" && op_str != "<") {
            return nullptr;
        }
        char op = op_str[0];
        if(type == RuleType::SIMPLE) {
            SimpleRule rule = SimpleRule(id, threshold, priority, sensor_id, op);
            return std::make_shared<SimpleRule>(rule);
        } else if(type == RuleType::STEP_DIFFERENCE) {
            StepDifferenceRule rule = StepDifferenceRule(id, threshold, priority, sensor_id, op);
            return std::make_shared<StepDifferenceRule>(rule);
        } else if(type == RuleType::STATEFUL) {
            if (!json.contains("consecutive_measurements") || !json["consecutive_measurements"].is_number_integer()) {
                return nullptr;
            }
            int consecutive_measurements = json["consecutive_measurements"].get<int>();
            StatefulRule rule = StatefulRule(id, threshold, priority, sensor_id, op, consecutive_measurements);
            return std::make_shared<StatefulRule>(rule);
        }
    }

}

// SimpleRule implementation
    SimpleRule::SimpleRule(const std::string &id, double threshold, std::optional<Priority> priority,
             const std::string &sensor_id, char op){
    id_ = id;
    threshold_ = threshold;
    priority_ = priority;
    sensor_id_ = sensor_id;
    op_ = op;
  }
  std::string SimpleRule::id() const { return id_; }
  RuleType SimpleRule::type() const { return RuleType::SIMPLE; }
  std::optional<Priority> SimpleRule::priority() const { return priority_; }

  EvaluationResult SimpleRule::evaluate(const std::string &sensor_id,
                            double value){
    if(sensor_id != sensor_id_) {
      return EvaluationResult::NOMINAL; // not applicable
    }
    if(op_ == '>') {
        return value > threshold_ ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
    }
    return value < threshold_ ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
  }

  // StepDifferenceRule implementation
  StepDifferenceRule::StepDifferenceRule(const std::string &id, double threshold, std::optional<Priority> priority,
              const std::string &sensor_id, char op){
    id_ = id;
    threshold_ = threshold;
    priority_ = priority;
    sensor_id_ = sensor_id;
    op_ = op;
  }
    std::string StepDifferenceRule::id() const { return id_; }
    RuleType StepDifferenceRule::type() const { return RuleType::STEP_DIFFERENCE; }
    std::optional<Priority> StepDifferenceRule::priority() const { return priority_; }
    std::string StepDifferenceRule::sensor_id() const { return sensor_id_; }
    EvaluationResult StepDifferenceRule::evaluate(const std::string &sensor_id,
                              double value){
      if(sensor_id != sensor_id_) {
        return EvaluationResult::NOMINAL;
      }
      if(!has_prev_value) {
        prev_value = value;
        has_prev_value = true;
        return EvaluationResult::NOMINAL; // not enough data to evaluate
       }

      double actual_threshold = prev_value + threshold_;
      prev_value = value;
      if(op_ == '>') {
        return value > actual_threshold ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
      }
      return value < actual_threshold ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL;
    }

    // StatefulRule implementation
    StatefulRule::StatefulRule(const std::string &id, double threshold, std::optional<Priority> priority,
              const std::string &sensor_id, char op, int consecutive_measurements){
    id_ = id;
    threshold_ = threshold;
    priority_ = priority;
    sensor_id_ = sensor_id;
    op_ = op;
    consecutive_measurements_ = consecutive_measurements;
    }
    std::string StatefulRule::id() const { return id_; }
    RuleType StatefulRule::type() const { return RuleType::STATEFUL; }
    std::optional<Priority> StatefulRule::priority() const { return priority_; }
    std::string StatefulRule::sensor_id() const { return sensor_id_; }
    EvaluationResult StatefulRule::evaluate(const std::string &sensor_id,
                              double value){
      if(sensor_id != sensor_id_) {
        return EvaluationResult::NOMINAL; 
      } 
      if(op_ == '>') {
        count = value > threshold_ ? count + 1 : 0;
      } else {
        count = value < threshold_ ? count + 1 : 0;
      }
      return count == consecutive_measurements_ ? EvaluationResult::ANOMALY : EvaluationResult::NOMINAL; 
    }

    // CorrelationRule implementation
    CorrelationRule::CorrelationRule(const std::string &id, std::optional<Priority> priority, const std::string &logic, 
    const std::vector<std::shared_ptr<Rule>> &parent_rules){
    id_ = id;
    priority_ = priority;
    logic_ = logic;
    parent_rules_ = parent_rules;
  }
    std::string CorrelationRule::id() const { return id_; }
    RuleType CorrelationRule::type() const { return RuleType::CORRELATION; }
    std::optional<Priority> CorrelationRule::priority() const { return priority_; }
    EvaluationResult CorrelationRule::evaluate(const std::string &sensor_id,
                              double value)  {
      if(logic_ == "AND") {
        for(const auto &rule : parent_rules_) {
          if(rule->evaluate(sensor_id, value) == EvaluationResult::NOMINAL) {
            return EvaluationResult::NOMINAL;
          }
        }
        return EvaluationResult::ANOMALY;
      }
        for(const auto &rule : parent_rules_) {
          if(rule->evaluate(sensor_id, value) == EvaluationResult::ANOMALY) {
            return EvaluationResult::ANOMALY;
          }
        }
      return EvaluationResult::NOMINAL;
    }
}