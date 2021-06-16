
const Course = require('../models/Course');
const { multipleMongooseToObject } = require('../../util/mongoose');

// contructor
class SiteController {
    // [GET] /
    index(req, res, next) {
        Course.find({})
            .then(courses => {
                res.render('home',{courses: multipleMongooseToObject(courses)});
            })
            .catch(next);
    }
    // [GET] /search
    show(req, res) {
        // render(file view js)
        res.render('search');
    }
}
module.exports = new SiteController();
