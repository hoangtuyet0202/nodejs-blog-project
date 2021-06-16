const Course = require('../models/Course');
const { multipleMongooseToObject } = require('../../util/mongoose');

// contructor
class MeController {
    // [GET] /me/stored/courses
    storedCourses(req, res, next) {
        Promise.all([
            Course.find({}).sortable(req),
            Course.countDocumentsDeleted()])
            .then(([course, deleteCount]) =>
                res.render('me/stored-courses', {
                    deleteCount,
                    course: multipleMongooseToObject(course)
                })
            )
            .catch(next);
        // res.json
    }

    // [GET] /me/trash/courses
    trashCourses(req, res, next) {
        Course.findDeleted({})
            .then(course => res.render('me/trash-courses',{course: multipleMongooseToObject(course)}))
            .catch(next);
    }
}
module.exports = new MeController();
