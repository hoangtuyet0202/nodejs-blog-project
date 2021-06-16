const newsRouter = require('./news');
const meRouter = require('./me');
const coursesRouter = require('./courses');
const siteRouter = require('./site');
// include all route
function route(app) {
    app.get('/news', newsRouter);
    app.use('/courses',coursesRouter);
    app.use('/me',meRouter);
    app.get('/', siteRouter);
}
module.exports = route;
